/* $Revision$ */
/* $Date$ */
/* This file (biddy-example-stuckatfaults.c) is a C++ file */
/* Author: Jovanka Dimkovska (jovanka.dimkovska@student.um.si), Robert Meolic (robert@meolic.com) */
/* This file has been released into the public domain by the authors. */

/* This example is compatible with Biddy v2.0 and laters */

/* COMPILE WITH (ADD -lgmp IF USING STATIC BIDDY LIBRARY): */
/* On Linux compile with: g++ -DUNIX -std=c++11 -o biddy-example-stuckatfaults biddy-example-stuckatfaults.c -I. -L./bin -lbiddy */
/* On Windows + MSYS2 compile with: g++ -DMINGW -std=c++11 -o biddy-example-stuckatfaults biddy-example-stuckatfaults.c -I. -L./bin -lbiddy */

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "biddy.h"

// these macros are helpful when using C++
#define Str2Var(x) (Biddy_GetVariable((Biddy_String)((std::string)x).c_str()))
#define EvaluateString(x) (Biddy_Eval2((Biddy_String)((std::string)x).c_str()))
#define SetValue(var,value) (Biddy_SetVariableValue(Str2Var(var),value?Biddy_GetConstantOne():Biddy_GetConstantZero()))
#define EvaluateFunction(f) (Biddy_Eval(f)?1:0)

using namespace std;

void writeStuckAtFaultsTable(Biddy_Edge f, vector<string> variables); // function prototype

int main() {
  Biddy_InitAnonymous(BIDDYTYPEOBDD);

  // operators: NOT (!), AND (*), XOR (^), OR (+), NOR (#), XNOR(-), NAND (@)
  string booleanExpression1 = "(x1 * x2) + x3";
  string booleanExpression2 = "(x1 + x3) * (x2 + x3)";

  Biddy_Edge f1 = EvaluateString(booleanExpression1);
  Biddy_Edge f2 = EvaluateString(booleanExpression2);

  // writing an optimized truth table is included in Biddy library
  // this is just a test to show this functionality

  cout << "Truth table for Boolean expression: " + booleanExpression1 << endl << endl;
  Biddy_PrintfTable(f1);
  cout << endl;

  cout << "Truth table for Boolean expression: " + booleanExpression2 << endl << endl;
  Biddy_PrintfTable(f2);
  cout << endl;

  // writing a truth table with all stuck-at-faults requires loop over all input combinations

  // first, create a vector of variables from all the existing Boolean functions
  vector<string> variables;
  for (unsigned int i=1; i<Biddy_VariableTableNum(); i++) {
    variables.push_back(Biddy_GetVariableName(Biddy_GetIthVariable(i)));
  }
  cout << "Variables in the existing Boolean functions:";
  for (string v: variables) cout << " " << v;
  cout << endl << endl;

  // then, call function that will loop over all input combinations and write the table

  cout << "Stuck-At-Faults table for Boolean expression: " + booleanExpression1 << endl << endl;
  writeStuckAtFaultsTable(f1,variables);
  cout << endl;

  cout << "Stuck-At-Faults table for Boolean expression: " + booleanExpression2 << endl << endl;
  writeStuckAtFaultsTable(f2,variables);
  cout << endl;

  Biddy_Exit();
}

/* loop over all combinations of the given variables and calculate Boolean function f */
void writeStuckAtFaultsTable(Biddy_Edge f, vector<string> variables)
{
  unsigned int numcombinations = 1;
  for (unsigned int i = 0; i < variables.size(); i++) numcombinations = 2 * numcombinations;
  for (string v: variables) cout << " " << v << " "; // write the first part of the legend
  cout << "| " << "z" << " |";
  for (string v: variables) cout << " " << v << "/0 " << v << "/1"; // write the second part of the legend
  cout << endl;
  for (int i = 0; i < numcombinations; i++) {
    unsigned int num = i;
    unsigned int pos = numcombinations;
    for (string v: variables) {
      pos = pos / 2;
      unsigned int value = (num>=pos)?1:0;
      if (value) num = num - pos;
      SetValue(v,value); // set a value to one variable
      cout << " " << setw(v.size()) << value << " ";
    }
    unsigned int z = EvaluateFunction(f); // evaluate function to get the result
    cout << "| " << z << " |";
    for (string v: variables) {
      unsigned int z0 = EvaluateFunction(Biddy_Restrict(f,Str2Var(v),0)); // evaluate function with stuck-at-zero
      unsigned int z1 = EvaluateFunction(Biddy_Restrict(f,Str2Var(v),1)); // evaluate function with stuck-at-one
      cout << " " << setw(v.size()+2) << z0 << " " << setw(v.size()+2) << z1;
    }
    cout << endl;
  }
}
