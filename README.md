# Using GitHub Actions with C++ and CMake

<div class="entry-content"><p>In this post I am going to provide a GitHub Actions configuration yaml file for C++ projects using CMake.</p>

<p><a href="https://github.com/features/actions">GitHub Actions</a> is a CI/CD infrastructure provided by GitHub. GitHub Actions
currently offers the following <a href="https://help.github.com/en/actions/automating-your-workflow-with-github-actions/virtual-environments-for-github-hosted-runners#supported-runners-and-hardware-resources">virtual machines (runners)</a>:</p>

<table>
  <thead>
    <tr>
      <th><strong>Virtual environment</strong></th>
      <th><strong>YAML workflow label</strong></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>Windows Server 2019</td>
      <td>windows-latest</td>
    </tr>
    <tr>
      <td>Ubuntu 18.04</td>
      <td>ubuntu-latest or ubuntu-18.04</td>
    </tr>
    <tr>
      <td>Ubuntu 16.04</td>
      <td>ubuntu-16.04</td>
    </tr>
    <tr>
      <td>macOS Catalina 10.15</td>
      <td>macos-latest</td>
    </tr>
  </tbody>
</table>

<p>Each virtual machine has the same hardware resources available:</p>

<ul>
  <li>2-core CPU</li>
  <li>7 GB of RAM memory</li>
  <li>14 GB of SSD disk space</li>
</ul>

<p>Each job in a workflow can run for up to <a href="https://help.github.com/en/actions/automating-your-workflow-with-github-actions/workflow-syntax-for-github-actions#usage-limits">6 hours</a> of execution time.</p>

<p>Unfortunately when I enabled GitHub Actions on a C++ project I was presented with this workflow:</p>

<div class="bogus-wrapper"><notextile><figure class="code"> <div class="CodeRay">
  <div class="code"><pre>./configure
make
make check
make distcheck
</pre></div>
</div>
 </figure></notextile></div>

<p>This is not something you can use with CMake though <img alt="smile" src="/images/smile.png" class="emoji"></p>

<h1 id="hello-world">Hello World</h1>

<p>I am going to build the following C++ hello world program:</p>

<div class="bogus-wrapper"><notextile><figure class="code"> <div class="CodeRay">
  <div class="code"><pre><span class="preprocessor">#include</span> <span class="include">&lt;iostream&gt;</span>

<span class="predefined-type">int</span> main()
{
  std::cout &lt;&lt; <span class="string"><span class="delimiter">"</span><span class="content">Hello world</span><span class="char">\n</span><span class="delimiter">"</span></span>;
}
</pre></div>
</div>
 </figure></notextile></div>

<p>With the following CMake project:</p>

<div class="bogus-wrapper"><notextile><figure class="code"> <div class="CodeRay">
  <div class="code"><pre>cmake_minimum_required(VERSION 3.16)

project(main)

add_executable(main main.cpp)

install(TARGETS main)

enable_testing()
add_test(NAME main COMMAND main)
</pre></div>
</div>
 </figure></notextile></div>

<p><strong>TL;DR</strong> see the project on <a href="https://github.com/Qannaf/GithubActions">GitHub</a>.</p>

<!-- more -->

<h1 id="build-matrix">Build Matrix</h1>

<p>I have started with the following build matrix:</p>

<div class="bogus-wrapper"><notextile><figure class="code"> <div class="CodeRay">
  <div class="code"><pre>name: CMake Build Matrix

on: [push]

jobs:
  build:
    name: ${&hairsp;{ matrix.config.name }&hairsp;}
    runs-on: ${&hairsp;{ matrix.config.os }&hairsp;}
    strategy:
      fail-fast: false
      matrix:
        config:
        - {
            name: "Windows Latest MSVC", artifact: "Windows-MSVC.tar.xz",
            os: windows-latest,
            build_type: "Release", cc: "cl", cxx: "cl",
            environment_script: "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Auxiliary/Build/vcvars64.bat"
          }
        - {
            name: "Windows Latest MinGW", artifact: "Windows-MinGW.tar.xz",
            os: windows-latest,
            build_type: "Release", cc: "gcc", cxx: "g++"
          }
        - {
            name: "Ubuntu Latest GCC", artifact: "Linux.tar.xz",
            os: ubuntu-latest,
            build_type: "Release", cc: "gcc", cxx: "g++"
          }
        - {
            name: "macOS Latest Clang", artifact: "macOS.tar.xz",
            os: macos-latest,
            build_type: "Release", cc: "clang", cxx: "clang++"
          }
</pre></div>
</div>
 </figure></notextile></div>

<h1 id="latest-cmake-and-ninja">Latest CMake and Ninja</h1>

<p>In the <a href="https://help.github.com/en/actions/automating-your-workflow-with-github-actions/software-installed-on-github-hosted-runners">software installed</a> on the runners page we
can see that CMake is installed on all runners, but with different versions:</p>

<table>
  <thead>
    <tr>
      <th><strong>Virtual environment</strong></th>
      <th><strong>CMake Version</strong></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>Windows Server 2019</td>
      <td>3.16.0</td>
    </tr>
    <tr>
      <td>Ubuntu 18.04</td>
      <td>3.12.4</td>
    </tr>
    <tr>
      <td>macOS Catalina 10.15</td>
      <td>3.15.5</td>
    </tr>
  </tbody>
</table>

<p>This would mean that one would have to limit the minimum CMake version to 3.12, or upgrade CMake.</p>

<p>CMake 3.16 comes with support for <a href="https://cmake.org/cmake/help/latest/command/target_precompile_headers.html">Precompile Headers</a>
and <a href="https://cmake.org/cmake/help/latest/prop_tgt/UNITY_BUILD.html">Unity Builds</a>, which help reducing build times.</p>

<p>Since CMake and Ninja have GitHub Releases, I decided to download those GitHub releases. <img alt="smile" src="/images/smile.png" class="emoji"></p>

<p>I used CMake as a scripting language, since the default scripting language for runners is <a href="https://help.github.com/en/actions/automating-your-workflow-with-github-actions/workflow-syntax-for-github-actions#using-a-specific-shell">different</a> (bash, and powershell).
CMake can execute processes, download files, extract archives.</p>

<div class="bogus-wrapper"><notextile><figure class="code"> <div class="CodeRay">
  <div class="code"><pre>- name: Download Ninja and CMake
  id: cmake_and_ninja
  shell: cmake -P {0}
  run: |
    set(ninja_version "1.9.0")
    set(cmake_version "3.16.2")

    message(STATUS "Using host CMake version: ${CMAKE_VERSION}")

    if ("${&hairsp;{ runner.os }&hairsp;}" STREQUAL "Windows")
      set(ninja_suffix "win.zip")
      set(cmake_suffix "win64-x64.zip")
      set(cmake_dir "cmake-${cmake_version}-win64-x64/bin")
    elseif ("${&hairsp;{ runner.os }&hairsp;}" STREQUAL "Linux")
      set(ninja_suffix "linux.zip")
      set(cmake_suffix "Linux-x86_64.tar.gz")
      set(cmake_dir "cmake-${cmake_version}-Linux-x86_64/bin")
    elseif ("${&hairsp;{ runner.os }&hairsp;}" STREQUAL "macOS")
      set(ninja_suffix "mac.zip")
      set(cmake_suffix "Darwin-x86_64.tar.gz")
      set(cmake_dir "cmake-${cmake_version}-Darwin-x86_64/CMake.app/Contents/bin")
    endif()

    set(ninja_url "https://github.com/ninja-build/ninja/releases/download/v${ninja_version}/ninja-${ninja_suffix}")
    file(DOWNLOAD "${ninja_url}" ./ninja.zip SHOW_PROGRESS)
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xvf ./ninja.zip)

    set(cmake_url "https://github.com/Kitware/CMake/releases/download/v${cmake_version}/cmake-${cmake_version}-${cmake_suffix}")
    file(DOWNLOAD "${cmake_url}" ./cmake.zip SHOW_PROGRESS)
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xvf ./cmake.zip)

    # Save the path for other steps
    file(TO_CMAKE_PATH "$ENV{GITHUB_WORKSPACE}/${cmake_dir}" cmake_dir)
    message("::set-output name=cmake_dir::${cmake_dir}")

    if (NOT "${&hairsp;{ runner.os }&hairsp;}" STREQUAL "Windows")
      execute_process(
        COMMAND chmod +x ninja
        COMMAND chmod +x ${cmake_dir}/cmake
      )
    endif()
</pre></div>
</div>
 </figure></notextile></div>

<h1 id="configure-step">Configure step</h1>

<p>Now that I have CMake and Ninja, all I have to do is configure the project like this:</p>

<div class="bogus-wrapper"><notextile><figure class="code"> <div class="CodeRay">
  <div class="code"><pre>- name: Configure
  shell: cmake -P {0}
  run: |
    set(ENV{CC} ${&hairsp;{ matrix.config.cc }&hairsp;})
    set(ENV{CXX} ${&hairsp;{ matrix.config.cxx }&hairsp;})

    if ("${&hairsp;{ runner.os }&hairsp;}" STREQUAL "Windows" AND NOT "x${&hairsp;{ matrix.config.environment_script }&hairsp;}" STREQUAL "x")
      execute_process(
        COMMAND "${&hairsp;{ matrix.config.environment_script }&hairsp;}" &amp;&amp; set
        OUTPUT_FILE environment_script_output.txt
      )
      file(STRINGS environment_script_output.txt output_lines)
      foreach(line IN LISTS output_lines)
        if (line MATCHES "^([a-zA-Z0-9_-]+)=(.*)$")
          set(ENV{${CMAKE_MATCH_1}&hairsp;} "${CMAKE_MATCH_2}")
        endif()
      endforeach()
    endif()

    file(TO_CMAKE_PATH "$ENV{GITHUB_WORKSPACE}/ninja" ninja_program)

    execute_process(
      COMMAND ${&hairsp;{ steps.cmake_and_ninja.outputs.cmake_dir }&hairsp;}/cmake
        -S .
        -B build
        -D CMAKE_BUILD_TYPE=${&hairsp;{ matrix.config.build_type }&hairsp;}
        -G Ninja
        -D CMAKE_MAKE_PROGRAM=${ninja_program}
      RESULT_VARIABLE result
    )
    if (NOT result EQUAL 0)
      message(FATAL_ERROR "Bad exit status")
    endif()
</pre></div>
</div>
 </figure></notextile></div>

<p>I have set the <code>CC</code> and <code>CXX</code> environment variables, and for MSVC, I had to run the <code>vcvars64.bat</code> script,
get all the environment variables, and set them for the CMake running script.</p>

<h1 id="build-step">Build step</h1>

<p>The build step involves running the CMake with <code>--build </code> parameter:</p>

<div class="bogus-wrapper"><notextile><figure class="code"> <div class="CodeRay">
  <div class="code"><pre>- name: Build
  shell: cmake -P {0}
  run: |
    set(ENV{NINJA_STATUS} "[%f/%t %o/sec] ")

    if ("${&hairsp;{ runner.os }&hairsp;}" STREQUAL "Windows" AND NOT "x${&hairsp;{ matrix.config.environment_script }&hairsp;}" STREQUAL "x")
      file(STRINGS environment_script_output.txt output_lines)
      foreach(line IN LISTS output_lines)
        if (line MATCHES "^([a-zA-Z0-9_-]+)=(.*)$")
          set(ENV{${CMAKE_MATCH_1}&hairsp;} "${CMAKE_MATCH_2}")
        endif()
      endforeach()
    endif()

    execute_process(
      COMMAND ${&hairsp;{ steps.cmake_and_ninja.outputs.cmake_dir }&hairsp;}/cmake --build build
      RESULT_VARIABLE result
    )
    if (NOT result EQUAL 0)
      message(FATAL_ERROR "Bad exit status")
    endif()
</pre></div>
</div>
 </figure></notextile></div>

<p>I set the <code>NINJA_STATUS</code> variable, to see how fast the compilation is in the respective runners.</p>

<p>For MSVC I reused the <code>environment_script_output.txt</code> script from the Configure step.</p>

<h1 id="run-tests-step">Run tests step</h1>

<p>This step calls <code>ctest</code> with number of cores passed as <code>-j</code> argument:</p>

<div class="bogus-wrapper"><notextile><figure class="code"> <div class="CodeRay">
  <div class="code"><pre>- name: Run tests
  shell: cmake -P {0}
  run: |
    include(ProcessorCount)
    ProcessorCount(N)

    execute_process(
      COMMAND ${&hairsp;{ steps.cmake_and_ninja.outputs.cmake_dir }&hairsp;}/ctest -j ${N}
      WORKING_DIRECTORY build
      RESULT_VARIABLE result
    )
    if (NOT result EQUAL 0)
      message(FATAL_ERROR "Running tests failed!")
    endif()
</pre></div>
</div>
 </figure></notextile></div>

<h1 id="install-pack-upload-steps">Install, pack, upload steps</h1>

<p>This steps involve running CMake with <code>--install</code>, then creating a <code>tar.xz</code> archive with CMake, and
uploading it as a build artifact.</p>

<div class="bogus-wrapper"><notextile><figure class="code"> <div class="CodeRay">
  <div class="code"><pre>- name: Install Strip
  run: ${&hairsp;{ steps.cmake_and_ninja.outputs.cmake_dir }&hairsp;}/cmake --install build --prefix instdir --strip

- name: Pack
  working-directory: instdir
  run: ${&hairsp;{ steps.cmake_and_ninja.outputs.cmake_dir }&hairsp;}/cmake -E tar cJfv ../${&hairsp;{ matrix.config.artifact }&hairsp;} .

- name: Upload
  uses: actions/upload-artifact@v1
  with:
    path: ./${&hairsp;{ matrix.config.artifact }&hairsp;}
    name: ${&hairsp;{ matrix.config.artifact }&hairsp;}
</pre></div>
</div>
 </figure></notextile></div>

<p>I didn’t use CMake as scripting language, since this just involves calling CMake with parameters, and the
default shells can handle this <img alt="smile" src="/images/smile.png" class="emoji"></p>

<h1 id="handling-releases">Handling Releases</h1>

<p>When you tag a release in git, you would also want the build artifacts promoted as releases:</p>

<div class="bogus-wrapper"><notextile><figure class="code"> <div class="CodeRay">
  <div class="code"><pre>git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0
</pre></div>
</div>
 </figure></notextile></div>

<p>The code to do this is below, gets triggered if the git refpath contains <code>tags/v</code>:</p>

<div class="bogus-wrapper"><notextile><figure class="code"> <div class="CodeRay">
  <div class="code"><pre>release:
  if: contains(github.ref, 'tags/v')
  runs-on: ubuntu-latest
  needs: build

  steps:
  - name: Create Release
    id: create_release
    uses: actions/create-release@v1.0.0
    env:
      GITHUB_TOKEN: ${&hairsp;{ secrets.GITHUB_TOKEN }&hairsp;}
    with:
      tag_name: ${&hairsp;{ github.ref }&hairsp;}
      release_name: Release ${&hairsp;{ github.ref }&hairsp;}
      draft: false
      prerelease: false

  - name: Store Release url
    run: |
      echo "${&hairsp;{ steps.create_release.outputs.upload_url }&hairsp;}" &gt; ./upload_url

  - uses: actions/upload-artifact@v1
    with:
      path: ./upload_url
      name: upload_url

publish:
  if: contains(github.ref, 'tags/v')
  name: ${&hairsp;{ matrix.config.name }&hairsp;}
  runs-on: ${&hairsp;{ matrix.config.os }&hairsp;}
  strategy:
    fail-fast: false
    matrix:
      config:
      - {
          name: "Windows Latest MSVC", artifact: "Windows-MSVC.tar.xz",
          os: ubuntu-latest
        }
      - {
          name: "Windows Latest MinGW", artifact: "Windows-MinGW.tar.xz",
          os: ubuntu-latest
        }
      - {
          name: "Ubuntu Latest GCC", artifact: "Linux.tar.xz",
          os: ubuntu-latest
        }
      - {
          name: "macOS Latest Clang", artifact: "macOS.tar.xz",
          os: ubuntu-latest
        }
  needs: release

  steps:
  - name: Download artifact
    uses: actions/download-artifact@v1
    with:
      name: ${&hairsp;{ matrix.config.artifact }&hairsp;}
      path: ./

  - name: Download URL
    uses: actions/download-artifact@v1
    with:
      name: upload_url
      path: ./
  - id: set_upload_url
    run: |
      upload_url=`cat ./upload_url`
      echo ::set-output name=upload_url::$upload_url

  - name: Upload to Release
    id: upload_to_release
    uses: actions/upload-release-asset@v1.0.1
    env:
      GITHUB_TOKEN: ${&hairsp;{ secrets.GITHUB_TOKEN }&hairsp;}
    with:
      upload_url: ${&hairsp;{ steps.set_upload_url.outputs.upload_url }&hairsp;}
      asset_path: ./${&hairsp;{ matrix.config.artifact }&hairsp;}
      asset_name: ${&hairsp;{ matrix.config.artifact }&hairsp;}
      asset_content_type: application/x-gtar
</pre></div>
</div>
 </figure></notextile></div>

<p>This looks complicated, but it’s needed since <code>actions/create-release</code> needs to be called only once, otherwise it will
fail. See <a href="https://github.com/actions/create-release/issues/14">issue #14</a>, <a href="https://github.com/actions/create-release/issues/27">issue #27</a> for
more information.</p>

<p>Even though you can use a workflow for 6 hours, the <code>secrets.GITHUB_TOKEN</code> expires in <a href="https://help.github.com/en/actions/automating-your-workflow-with-github-actions/authenticating-with-the-github_token#about-the-github_token-secret">one hour</a>. You can either create a personal token, or
upload the artifacts manually to the release. See <a href="https://github.community/t5/GitHub-Actions/error-Bad-credentials/td-p/33500">this</a> GitHub community
thread for more information.</p>

<h1 id="closing">Closing</h1>

<p>Enabling GitHub Actions on your CMake project is as easy at creating a <code>.github/workflows/build_cmake.yml</code> file with the content from
<a href="https://github.com/Qannaf/GithubActions/blob/main/.github/workflows/build_cmake.yml" class="download">build_cmake.yml</a>.</p>

<p>You can see the GitHub Actions at my <a href="https://github.com/Qannaf/GithubActions">Hello World</a> GitHub project.</p>
</div>