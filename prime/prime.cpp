// prime.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
/*
 * Sieve of Eratosthenes
 * Anders Karlsson 2015-2017
 */

#include <algorithm>
#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <numeric> // iota
#include <string>
#include <vector>
#include <cctype>
#include <iterator>
#include <optional>

namespace
{
  static bool trace = false;
}

bool verifyFunctionality();

std::vector<long long> generatePrimes();
std::pair<long long, long long>
  decimalToFraction(const std::string& number, std::vector<long long>& primes, const bool output = true);
std::map<long long, long long>
  factorizeNumber(const std::string& number, std::vector<long long>& primes, const bool output = true);
void printSyntax();

int main(int argc, char* argv[])
{
  std::string number;

  assert(verifyFunctionality());

  try
  {
    bool calculatePrimeNumber = false;
    if (argc == 1)
    {
      std::cout << "Enter an integer number to factorize into prime numbers:" << std::flush;
      std::getline(std::cin, number);
      calculatePrimeNumber = (number.find('.') == std::string::npos);
    }
    else
    {
      while (--argc)
      {
        std::string param(*++argv);
        if (!param.empty())
        {
          if (param.find('.') != std::string::npos)
          {
            number = param;
          }
          else if (isdigit(param[0]) && stoll(param) != 0)
          {
            calculatePrimeNumber = true;
            number = param;

            if (std::stoll(number) > std::numeric_limits<long long>::max() - 1)
            {
              std::cerr << "too large int" << std::endl;
              return -2;
            }
          }
          else if (param[0] == '-' && param.length() > 1)
          {
            trace = (std::tolower(param[1]) == 't' || std::tolower(param[1]) == 'v');
          }
          else
          {
            std::cout << "Invalid command line option: '" << param << "'" << std::endl;
            printSyntax();
            return -1;
          }
        }
      }
    }

    // generate some primes using Eratosthenes method
    auto primes = generatePrimes();

    if (!calculatePrimeNumber) // from decimal to fraction e.g. 2.25 => 2 1/4
    {
      decimalToFraction(number, primes);
    }
    else
    {
      factorizeNumber(number, primes);
    }
  }
  catch (const std::invalid_argument& ex)
  {
    std::cerr << "please specify an integer value " << ex.what() << std::endl;
  }
  catch (const std::out_of_range& ex) // for ridiculuous numbers
  {
    std::cerr << "too large int " << ex.what() << std::endl;
  }
  return 0;
}

void printSyntax()
{
  using std::cout;
  using std::endl;

  cout << "Valid command line options are C>prime {n}|{x.y} [-t|-v]" << endl;
  cout << "n   == integer != 0" << endl;
  cout << "x.y == double value != 0.0" << endl;
  cout << "t   == trace" << endl << endl;
  cout << "E.g." << endl;
  cout << "  C>prime 1234 will give 2*617 (prime numbers)" << endl;
  cout << "  C>prime 12.25 will give 12 1/4 (fractions)" << endl;
}

/**
 * Good old Eratosthenes way of calculating prime numbers, the method can
 * briefly be described as having a 2-dimensional table of numbers e.g. N x N; 1
 * 2 3... x 1 2 3 ... then multiplying the numbers with one another -- any
 * number that is not a product, is a prime number (1 is excluded since it isn't a prime number)
 *
 * @sa Euclid's lemma: If a prime divides the product of ab of two integers a and b,
 * then p must divide at least of of those integers a and b
 *
 * Example: find all prime numbers 1-10
 *
 *  1 2 3 4 5 6 7 8 9
 * 1x x x x x x x x x
 * 2x 4 6 8 .........  eliminates 4,6,8      "..." indicates a product >10
 * 3x 6 9 ...          eliminates 9
 * 4x 8 ...
 * 5x ...
 * 6x ...
 * 7x ...
 * 8x ...
 * 9x ...
 *
 * So from the original numbers 1,2,3,4,5,6,7,8,9 we remove the calculated
 * 4,6,8,9 and then 1,2,3,5,7 are left.
 */
std::vector<long long> generatePrimes()
{
  auto start = std::chrono::system_clock::now();
  std::vector<long long> primes;
  const auto primeCandidates = 999'999U;
  std::vector<long long> candidates(primeCandidates);
  std::iota(std::begin(candidates), std::end(candidates), 1ll); // vector with 1,2, ... primeCandidates

  for (auto i = 2U; i < candidates.size(); ++i)
  {
    for (auto j = 2U; j < candidates.size(); ++j)
    {
      if (i * j <= primeCandidates)
      {
        candidates[i * j - 1] = 0ll;
      }
      else
      {
        break; // quit since all products after will be larger
      }
    }
  }

  for (long long n : candidates)
  {
    if (n > 1) // 1 not a prime number
    {
      primes.emplace_back(n);
    }
  }

  if (trace)
  {
    using namespace std::chrono;
    auto stop = system_clock::now();

    std::cout << "Calculated " << primes.size() << " prime numbers using 'Sieve of Eratosthenes'"
              << " which took " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count()
              << " ms" << std::endl;
    std::cout << "Last ten::";
    for (auto it = primes.rbegin(); it != primes.rbegin() + 10; ++it)
    {
      std::cout << *it << " ";
    }
    std::cout << std::endl;
  }

  return primes;
}

/**
 * Given a number, calculate the prime numbers in it.
 */
std::vector<long long> divideWithPrimes(long long number, const std::vector<long long>& primes)
{
  std::vector<long long> factors;

  if (number == 1)
  {
    factors.push_back(1);
    return factors;
  }

  for (auto n : primes)
  {
    while (number % n == 0 && number != 1)
    {
      factors.push_back(n);
      number /= n;
    }
  }

  return factors;
}

/**
 * Take a sorted vector of prime numbers and prints them to stdout.
 */

void printFactors(std::vector<long long>& factors)
{
  int count = 0;
  std::cout << std::setw(3);
  for (auto i : factors)
  {
    std::cout << ((count++ == 0) ? " " : "*") << i;
  }
  std::cout << std::endl;
}

/**
 * Convert a decimal value to int e.g. 0.25 --> 1/4
 */

bool extractNumeratorDenominator(std::string line, long long& numerator, long long& denominator)
{
  bool ok = true;
  try
  {
    auto pos = line.find('.');

    if (pos != 0 && line.length() - pos >= 9)
    {
      throw std::out_of_range("line too long");
    }

    int m = (pos != 0) ? std::stoi(line.substr(0, pos)) : 0; // 2.25 -> m = 2   .25 -> m = 0
    line = line.substr(pos);

    auto len = line.length(); // .12 = 3

    denominator = 1; // namnare
    for (size_t i = 1; i < len; ++i)
    {
      denominator *= 10; // .12 --> 12/100
    }

    numerator = stoi(line.substr(1, len - 1)); // taljare.
    numerator += denominator * m;
  }
  catch (std::out_of_range& ex)
  {
    std::cout << "number has too many digits " << ex.what() << std::endl;
    ok = false;
  }
  return ok;
}

/**
 * Given factors, calculate product
 */
long long calculateProduct(std::vector<long long>& factors)
{
  long long n = 1;
  for (auto i : factors)
  {
    n *= i;
  }
  return n;
}

/**
 * given two vectors, remove common elements in the vectors.
 *
 * e.g.
 * {1,2,3,3}
 * {2,3,4,5}
 * --> {1,3} {4,5}
 */
std::pair<std::vector<long long>, std::vector<long long>>
  removeCommonNumbers(std::vector<long long>& numerator, std::vector<long long>& denominator)
{
  std::vector<long long> inter;

  // determine common numbers, put common numbers in 'inter'
  std::set_intersection(numerator.begin(), numerator.end(), denominator.begin(), denominator.end(), std::back_inserter(inter));

  if (trace)
  {
    std::cout << "remove common numbers, use an intersection for this (c++ algorithm)" << std::endl
              << std::endl;

    std::cout << "  intersection:";
    for (auto i : inter)
    {
      std::cout << i << " ";
    }
    std::cout << std::endl;
  }

  // are there any common numbers? if not we return vectors as is.
  if (inter.size() != 0)
  {
    // remove elements from numerator vector
    std::vector<long long> leftn;
    std::set_difference(
      numerator.begin(), numerator.end(), inter.begin(), inter.end(), std::inserter(leftn, leftn.begin()));

    if (leftn.size() == 0)
    {
      leftn.push_back(1);
    }

    if (trace)
    {
      std::cout << "  -------------" << std::endl;
      std::cout << "  new numerator:";
      for (auto i : leftn)
      {
        std::cout << i << " ";
      }
      std::cout << std::endl;
    }

    // remove elements from denominator vector
    std::vector<long long> leftd;
    set_difference(
      denominator.begin(), denominator.end(), inter.begin(), inter.end(), std::inserter(leftd, leftd.begin()));

    if (leftd.size() == 0)
    {
      leftd.push_back(1);
    }

    if (trace)
    {
      std::cout << "  new denominator:";
      for (auto i : leftd)
      {
        std::cout << i << " ";
      }
      std::cout << std::endl << std::endl;
    }

    // return new vectors.
    return std::make_pair(leftn, leftd);
  }
  else
  {
    return std::make_pair(numerator, denominator);
  }
}

//////////////////////////////////////////////////////////////////
// main functions
//////////////////////////////////////////////////////////////////

std::pair<long long, long long> decimalToFraction(const std::string& number, std::vector<long long>& primes, const bool output)
{
  long long numerator = 1;
  long long denominator = 1;

  // given .12 create an integer version of it, i.e. 12/100
  if (!extractNumeratorDenominator(number, numerator, denominator))
  {
    return std::make_pair(0, 0);
  }

  if (trace)
  {
    std::cout << "remove decimal point by multiplication" << std::endl;
    std::cout << "  " << numerator << "/" << denominator << std::endl << std::endl;
  }

  // divide numerator and denominator into primes
  auto factorsnumerator = divideWithPrimes(numerator, primes);
  auto factorsdenominator = divideWithPrimes(denominator, primes);

  if (trace)
  {
    std::cout << "calculate prime numbers for numerator and denominator" << std::endl;
    printFactors(factorsnumerator);
    std::cout << "--------------------------" << std::endl;
    printFactors(factorsdenominator);
    std::cout << std::endl;
  }

  // given the vectors of primes, remove common ones from numerator and
  // denominator
  auto [num, den] = removeCommonNumbers(factorsnumerator, factorsdenominator);

  if (trace)
  {
    printFactors(num);
    std::cout << "--------------------------" << std::endl;
    printFactors(den);
    std::cout << std::endl;
  }

  // after removing common numbers, recalculate denominator and numerator
  auto t = calculateProduct(num);
  auto n = calculateProduct(den);

  if (output)
  {
    if (t > n)
    {
      // 9/4
      // 9/4 = 2
      // 9 - 2*4 = 1
      // 2 1/4
      std::cout << number << " = " << t << "/" << n << " ==> " << (t / n) << " " << (t - (t / n) * n) << "/"
                << n << std::endl;
    }
    else
    {
      std::cout << number << " = " << t << "/" << n << std::endl;
    }
  }
  return std::make_pair(t, n);
}

//////////////////////////////////////////////////////////////////

std::map<long long, long long> factorizeNumber(const std::string& number, std::vector<long long>& primes, const bool output)
{
  auto m = std::stoll(number);

  if (output)
  {
    std::cout << std::endl << std::setw(10) << m << " = ";
  }

  auto factors = divideWithPrimes(m, primes);
  std::map<long long, long long> factorsWithExp;
  for (long long i : factors)
  {
    auto it = factorsWithExp.find(i);
    if (it != factorsWithExp.end())
    {
      factorsWithExp[i]++;
    }
    else
    {
      factorsWithExp[i] = 1;
    }
  }

  if (output)
  {
    int count = 0; // number of factors printed
    for (auto n : factorsWithExp)
    {
      if (count++ > 0)
      {
        std::cout << "* ";
      }

      if (n.second != 1)
      {
        std::cout << n.first << "^" << n.second << " ";
      }
      else
      {
        std::cout << n.first;
      }
    }

    std::cout << std::endl;
  }

  return factorsWithExp;
}

/**
 * sanity check to verify that nothing is broken after any changes, it is always run at program start.
 */

bool verifyFunctionality()
{
  std::vector<long long> primes = generatePrimes();
  auto result1230 = divideWithPrimes(1230, primes);
  if (result1230.size() != 4)
  {
    std::cerr << "Invalid number of primes 1230" << std::endl;
    return false;
  }
  if (calculateProduct(result1230) != 1230)
  {
    std::cerr << "Invalid factors 1230" << std::endl;
    return false;
  }
  auto result1231 = divideWithPrimes(1231, primes);
  if (result1231.size() != 1)
  {
    std::cerr << "Invalid number of primes 1231" << std::endl;
    return false;
  }
  if (calculateProduct(result1231) != 1231)
  {
    std::cerr << "Invalid factors 1231" << std::endl;
    return false;
  }

  const bool output = false;
  auto [t, n] = decimalToFraction(std::string("0.12"), primes, output);
  if (t != 3)
  {
    std::cerr << "Invalid denominator" << std::endl;
    return false;
  }
  if (n != 25)
  {
    std::cerr << "Invalid numerator" << std::endl;
    return false;
  }

  auto m = factorizeNumber(std::string("13112"), primes, output);
  if (m[2] != 3 || m[11] != 1 || m[149] != 1)
  {
    std::cerr << "factorizing failed" << std::endl;
  }

  return true;
}
