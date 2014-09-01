/*
 * colsum.cpp
 *
 *  Created on: May 20, 2013
 *      Author: Carsten Kemena
 *
 *   Copyright 2013 Carsten Kemena
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


// C headers
#include <cstdlib>

// C++ headers
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <limits>

// boost header
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include "Version.hpp"

using namespace std;

using boost::lexical_cast;
using boost::bad_lexical_cast;
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

/**
 * \brief Turns a string into column ids.
 *
 * The string can contain "," to separate columns and "-" to denote a range of columns.
 * @param column_str The string with the encoded columns.
 * @param[out] col_ids The columns id extracted from the column_str, sorted ascending.
 */
void
str2col_id(const string &column_str, vector<int> &col_ids)
{
	const char *str=column_str.c_str();
	if ((!isdigit(str[0])) || (!isdigit(str[column_str.size()-1])))
		return;
	bool last_digit=true;
	for (size_t i =0; i<column_str.size(); ++i)
	{
		if (!isdigit(str[i]))
		{
			if (last_digit)
				last_digit=false;
			else
				return;
		}
		else
			last_digit=true;
	}
	int start, end, i;
	start=end=atoi(str);

	while (*str != '\0')
	{
		if (*str == ',')
		{
			for (i=start; i<=end; ++i)
				col_ids.push_back(i-1);
			start=end=atoi(++str);
		} else if (*str == '-')
			end=atoi(++str);
		++str;
	}
	for (i=start; i<=end; ++i)
		col_ids.push_back(i-1);

	sort(col_ids.begin(), col_ids.end());
}


void
line2tex(const string &line, ostream* out_p, const string &delim, vector<int> &colIds)
{
	std::vector<std::string> tokens = split(line, delim);

	if (colIds.empty())
	{
		(*out_p) << tokens[0];
		size_t len=tokens.size();
		for (size_t i=1; i<len; ++i)
			(*out_p) << " & " << tokens[i];
	}
	else
	{
		(*out_p) << tokens[colIds[0]];
		size_t len=colIds.size();
		for (size_t i=1; i<len; ++i)
			(*out_p) << " & " << tokens[colIds[i]];
	}
	(*out_p) << "\\\\\n";
}

int
main(int argc, char *argv[])
{
	string in_f;
	string out_f;
	string col_str;
	string delim;
	bool no_header;
	vector<string> patterns;
	po::options_description general_opts("General options");
	general_opts.add_options()
		("help,h", "Produces this help message")
		("in,i", po::value<string>(&in_f), "Input file")
		("out,o", po::value<string>(&out_f), "Output file")
		("no-header,n", po::value<bool>(&no_header)->default_value(false)->zero_tokens(), "Do not print column identifier")
	;

	po::options_description table_opts("Input format options");
	table_opts.add_options()
		("delimiter,d", po::value<string>(&delim)->default_value("\t"), "Delimiter to use")
		("columns,c", po::value<string>(&col_str), "The columns to use")
	;

	po::options_description all("csv2tex (" + version + ") Copyright (C) 2014  Carsten Kemena\nThis program comes with ABSOLUTELY NO WARRANTY;\n\nAllowed options are displayed below.");
	all.add(general_opts).add(table_opts);
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

	vector<int> col_ids;
	if (!col_str.empty())
		str2col_id(col_str, col_ids);

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

	string line;

	// table
	(*out_p) << "\\begin{tabular}{";
	for (size_t i=0; i<col_ids.size(); ++i)
		(*out_p) << "r";
	(*out_p) <<"}\n";

	if (!no_header)
	{
		getline(*in_p, line);
		(*out_p) << "\\hline" << "\n";
		line2tex(line, out_p, delim, col_ids);
		(*out_p) << "\\hline" << "\n";
	}


	while (getline(*in_p, line))
	{
		line2tex(line, out_p, delim, col_ids);
	}


	// end table
	if (!no_header)
		(*out_p) << "\\hline" << "\n";
	(*out_p) << "\\end{tabular}\n";




	// close files & release memory
	if (!in_f.empty())
		in_F.close();
	if (!out_f.empty())
		out_F.close();

	exit(EXIT_SUCCESS);

}

