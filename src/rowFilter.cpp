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
 * This file is part of LittleToolBox.
 *
 * LittleToolBox is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LittleToolBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LittleToolBox.  If not, see <http://www.gnu.org/licenses/>.
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


int
main(int argc, char *argv[])
{
	string in_f;
	string out_f;
	int cols;
	string delim;
	string filterFunction;
	po::options_description general_opts("General options");
	general_opts.add_options()
		("help,h", "Produces this help message")
		("in,i", po::value<string>(&in_f)->required(), "Input file")
		("out,o", po::value<string>(&out_f)->required(), "Output file")
		("col,c", po::value<int>(&cols)->required(), "Columns in file")
		("delimiter,d", po::value<string>(&delim)->default_value("\t"), "Delimiter")
		("filter,f", po::value<string>(&filterFunction)->required(), "The filterFunction to use")
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

	double threshold;
	std::function<double(const double&, const double&)> filterFunc;

	if (filterFunction[0] == '>')
	{
		if (filterFunction[1] == '=')
		{
			filterFunc = std::greater_equal<double>();
			threshold = stod(filterFunction.substr(2));
		}
		else
		{
			filterFunc = std::greater<double>();
			threshold = stod(filterFunction.substr(1));
		}
	}
	if (filterFunction[0] == '<')
	{
		if (filterFunction[1] == '=')
		{
			filterFunc = std::less_equal<double>();
			threshold = stod(filterFunction.substr(2));
		}
		else
		{
			filterFunc = std::less<double>();
			threshold = stod(filterFunction.substr(1));
		}
	}

	if ((filterFunction[0] == '=') && (filterFunction[1] == '='))
	{
		filterFunc = std::equal_to<double>();
		threshold = stod(filterFunction.substr(2));
	}

	--cols;

	ifstream inS(in_f);
	ofstream outS(out_f);
	string line;
	while (getline(inS, line))
	{
		std::vector<std::string> tokens = split(line, delim);
		cout << stod(tokens[cols]) << endl;
		if (filterFunc(stod(tokens[cols]), threshold))
			outS << line << "\n";
	}
	outS.close();
	inS.close();


	return EXIT_SUCCESS;
}
