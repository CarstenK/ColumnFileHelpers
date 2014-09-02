/*
 * rowFilter.cpp
 *
 *  Created on: Jun 21, 2014
 *      Author: Carsten Kemena
 *
 *   Copyright 2014 Carsten Kemena
 *   Email CarstenK[at]posteo.de
 *
 *
 * This file is part of ColumnFileHelpers.
 *
 * ColumnFileHelpers is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ColumnFileHelpers is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ColumnFileHelpers.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <vector>
#include <string>
#include <fstream>
#include <functional>
#include <cctype>

// boost headers
#include <boost/program_options.hpp>

#include "Version.hpp"

using namespace std;

namespace po = boost::program_options;
using namespace std::placeholders;


std::vector<std::string>
split(const std::string &s, const std::string &delimiters)
{
	size_t start=0;
	size_t end=s.find_first_of(delimiters);
	std::vector<std::string> output;
	size_t len=s.size();
	while (start != len)
	{
		if (end-start >0)
			output.emplace_back(s.substr(start, end-start));

		if (end == std::string::npos)
			break;
		start=end+1;
		end = s.find_first_of(delimiters, start);
	}

	return output;
}

/*
bool
columCompare(int columnId, std::function<double(const double&, const double&)> &compFunction, string value, const vector<string> &tokens)
{
	return compFunction(tokens[columnId], value);
}*/


bool
columCompare(int columnId,  std::function<double(const double&, const double&)> &compFunction, double value, const vector<string> &tokens)
{
	return compFunction(stod(tokens.at(columnId)), value);
}


int
main(int argc, char *argv[])
{
	string in_f;
	string out_f;
	string delim;
	vector<string> filterFunctions;
	po::options_description general_opts("General options");
	general_opts.add_options()
		("help,h", "Produces this help message")
		("in,i", po::value<string>(&in_f), "Input file")
		("out,o", po::value<string>(&out_f), "Output file")
		("delimiter,d", po::value<string>(&delim)->default_value("\t", "TAB"), "Delimiter")
		("filter,f", po::value<vector<string> >(&filterFunctions)->multitoken()->required(), "The filterFunction to use")
	;

	po::options_description all("rowFilter (" + version + ") Copyright (C) 2014  Carsten Kemena\nThis program comes with ABSOLUTELY NO WARRANTY;\n\nAllowed options are displayed below.");
	all.add(general_opts);
	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(all).run(), vm);
	if (vm.count("help"))
	{
		cout << all<< "\n";
		return EXIT_SUCCESS;
	}

	try
	{
		po::notify(vm);
	}
	catch(std::exception &e)
	{
		std::cerr << "Error parsing commandline: " << e.what() << endl;
		std::cerr << "Use -h/--help for option information" << endl;
		exit(EXIT_SUCCESS);
	}

	// parse filter functions
	vector<std::function<bool(const vector<string>&)> > vecFuncts;
	for (string &filterToken : filterFunctions)
	{
		double threshold;
		std::function<double(const double&, const double&)> filterFunc;
		size_t pos = filterToken.find_first_of("=><");
		size_t colId = std::stoi(filterToken);
		filterToken = filterToken.substr(pos);

		if (filterToken[0] == '>')
		{
			if (filterToken[1] == '=')
			{
				filterFunc = std::greater_equal<double>();
				threshold = stod(filterToken.substr(2));
			}
			else
			{
				filterFunc = std::greater<double>();
				threshold = stod(filterToken.substr(1));
			}
		}
		if (filterToken[0] == '<')
		{
			if (filterToken[1] == '=')
			{
				filterFunc = std::less_equal<double>();
				threshold = stod(filterToken.substr(2));
			}
			else
			{
				filterFunc = std::less<double>();
				threshold = stod(filterToken.substr(1));
			}
		}

		if ((filterToken[0] == '=') && (filterToken[1] == '='))
		{
			filterFunc = std::equal_to<double>();
			threshold = stod(filterToken.substr(2));
		}

		--colId;
		vecFuncts.push_back(std::bind (columCompare, colId, filterFunc, threshold, _1));
	}
	//ifstream inS(in_f);
	//ofstream outS(out_f);
	string line;
	int lineNum = 0;

	ifstream in_F;
	istream* in_p;

	if(in_f.empty())
		in_p = &cin;
	else
	{
		in_F.open(in_f.c_str(), ifstream::in);
		if (in_F.good())
			in_p = &in_F;
		else
		{
			cerr << "Error! Could not open file " << in_f << endl;
			exit(EXIT_FAILURE);
		}
	}


	ofstream out_F;
	ostream* out_p;
	if(out_f.empty())
		out_p = &cout;
	else
	{
		out_F.open(out_f.c_str(), ifstream::out);
		if (out_F.good())
			out_p = &out_F;
		else
		{
			cerr << "Error! Could not open file " << out_f << endl;
			exit(EXIT_FAILURE);
		}
	}



	try {
		while (getline((*in_p), line))
		{
			++lineNum;
			if (line.empty())
				continue;
			std::vector<std::string> tokens = split(line, delim);

			bool passedThreshold = true;
			for (auto &functs : vecFuncts)
				passedThreshold &= functs(tokens);
			if (passedThreshold)
				(*out_p) << line << "\n";
		}
	}
	catch (exception &e)
	{
		cerr << "Error occurred when parsing line " << lineNum << "!" << endl;
	}

	if (!in_f.empty())
		in_F.close();
	if (!out_f.empty())
		out_F.close();

	return EXIT_SUCCESS;
}
