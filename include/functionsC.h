#pragma once
#ifndef FUNCTIONSC_H
#define FUNCTIONSC_H
#include<iostream>

namespace functionsC
{
	int Cube(const int& x);

	class BankAccount
	{
	public:
		int balance;
		BankAccount(const int& balance = 1) :balance{ balance } { }
	};
}

#endif // !FUNCTIONSC_H
