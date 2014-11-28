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


/**
 * \brief splits a line into tokens
 * @param s The string to plit
 * @param delimiters The delimiters to use
 * @return A vector of tokens.
 */
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
 * \brief Replaces characters that have special meaning in latex with the backslashed version.
 * @param line The line in which to replace the characters
 */
void
replace(string &line)
{
	string characters = "_%#";
	size_t pos = 0;
	string re;
	for (char c : characters)
	{
		while ((pos=line.find(c, pos)) != string::npos)
		{
			re = "\\";
			re.push_back(c);
			line.replace(pos,1,re);
			pos+=2;
		}
		pos = 0;
	}
}


/**
 * \brief Turns a string into column ids.
 *
 * The string can contain "," to separate columns and "-" to denote a range of columns.
 * @param column_str The string with the encoded columns.
 * @param[out] colIds The columns id extracted from the column_str.
 */
int
str2col_id(const string &column_str, vector<int> &colIds)
{
	const char *str=column_str.c_str();
	if ((!isdigit(str[0])) || (!isdigit(str[column_str.size()-1])))
		return -1;
	bool last_digit=true;
	for (size_t i =0; i<column_str.size(); ++i)
	{
		if (!isdigit(str[i]))
		{
			if (last_digit)
				last_digit=false;
			else
				return -1;
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
				colIds.push_back(i-1);
			start=end=atoi(++str);
		} else if (*str == '-')
			end=atoi(++str);
		++str;
	}
	for (i=start; i<=end; ++i)
		colIds.push_back(i-1);

	int maximum=-1;
	for (const auto value: colIds)
	{
		if (value > maximum)
			maximum = value;
	}
	return maximum;
}


/**
 * Turns a line into a latex table line
 * @param line The line to convert.
 * @param outP The stream to print to.
 * @param delim The delimiter to use.
 * @param colIds The column ids to use.
 */
void
line2tex(string &line, ostream* outP, const string &delim, vector<int> &colIds, size_t maximum)
{
	if (line.empty())
		return;
	replace(line);
	std::vector<std::string> tokens = split(line, delim);
	if (tokens.size() <= maximum)
		throw std::runtime_error("Not enough columns");
	if (colIds.empty())
	{
		(*outP) << tokens[0];
		size_t len=tokens.size();
		for (size_t i=1; i<len; ++i)
			(*outP) << " & " << tokens[i];
	}
	else
	{
		(*outP) << tokens[colIds[0]];
		size_t len=colIds.size();
		for (size_t i=1; i<len; ++i)
			(*outP) << " & " << tokens[colIds[i]];
	}
	(*outP) << "\\\\\n";
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
		("no-header,n", po::value<bool>(&no_header)->default_value(false)->zero_tokens(), "Do not print a headline")
	;

	po::options_description table_opts("Input format options");
	table_opts.add_options()
		("delimiter,d", po::value<string>(&delim)->default_value("\t", "TAB"), "Delimiter to use")
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

	vector<int> colIds;
	size_t maximum = 0;
	if (!col_str.empty())
		maximum = str2col_id(col_str, colIds);

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
	ostream* outP;
	if(out_f.empty())
		outP = &cout;
	else
	{
		out_F.open(out_f.c_str(), ifstream::out);
		if (out_F.good())
			outP = &out_F;
		else
		{
			cerr << "Error! Could not open file " << out_f << endl;
			exit(EXIT_FAILURE);
		}
	}

	string line;
	getline(*in_p, line);
	size_t nColumns;
	if (colIds.empty())
	{
		vector<string> tokens = split(line, delim);
		nColumns = tokens.size();
		maximum = nColumns-1;
	}
	else
		nColumns = colIds.size();

	// table
	(*outP) << "\\begin{tabular}{";
	for (size_t i=0; i<nColumns; ++i)
		(*outP) << "r";
	(*outP) <<"}\n";
	size_t lineNum = 1;
	try
	{
		if (!no_header)
		{
			(*outP) << "\\hline" << "\n";
			line2tex(line, outP, delim, colIds, maximum);
			(*outP) << "\\hline" << "\n";
		}
		else
			line2tex(line, outP, delim, colIds, maximum);


		while (getline(*in_p, line))
		{
			++lineNum;
			line2tex(line, outP, delim, colIds, maximum);
		}
	}
	catch(std::exception &e)
	{
		std::cerr << "An error occured when reading line " << lineNum << ": " << e.what()<< endl;
		exit(EXIT_FAILURE);
	}

	// end table
	if (!no_header)
		(*outP) << "\\hline" << "\n";
	(*outP) << "\\end{tabular}\n";




	// close files & release memory
	if (!in_f.empty())
		in_F.close();
	if (!out_f.empty())
		out_F.close();

	exit(EXIT_SUCCESS);

}

