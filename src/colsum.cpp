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
#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include "Version.hpp"

using namespace std;

namespace po = boost::program_options;

double my_max(double a, double b)
{
	return (a>b? a : b);
}

double my_min(double a, double b)
{
	return (a<b? a : b);
}

double my_sum(double a, double b)
{
	return (a+b);
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


/**
 * Splits the lines into the needed columns.
 * @param compl_line The complete line.
 * @param delim The delimiter to separate the columns.
 * @param col_ids The vector with the column id to use.
 * @param[out] values The vector of same length as col_ids to which the values will be added.
 */

void
parse_line(const string &line, char delim, const vector<int> &col_ids, double *values, size_t n_line, bool merge, double (*analysis_func)(double, double))
{
	//size_t start=0;
	size_t len = line.length();
	int curr_col=0;
	size_t line_pos=0;
	const char *line_c=line.c_str();
	const char *prev_col = line.c_str();
	int n_cols=col_ids.size();
	int i=0;
	while ((i<n_cols) && (line_pos<len))
	{
		prev_col=&line_c[line_pos];
		//start=line_pos;
		while ((line_pos != len) && (line_c[line_pos] != delim))
			++line_pos;
		if (curr_col==col_ids[i])
		{
			values[i] = analysis_func(values[i], std::stod(prev_col));//, line_pos-start));
			++i;
		}
		++line_pos;
		if (merge)
		{
			while ((line_pos != len) && (line_c[line_pos] == delim))
				++line_pos;
		}

		++curr_col;

	}

	if (i<n_cols)
	{
		fprintf(stderr, "Error! Line %li does not contain enough columns!\n", n_line);
		exit(EXIT_FAILURE);
	}
}


int
main(int argc, char *argv[])
{
	string in_f;
	string out_f;
	string col_str;
	short precision;
	char delim;
	int k;
	bool average;
	bool merge;
	bool no_header;
	int skip_lines;
	bool sum;
	bool max_value, min_value;
	vector<string> patterns;
	po::options_description general_opts("General options");
	general_opts.add_options()
		("help,h", "Produces this help message")
		("in,i", po::value<string>(&in_f), "Input file")
		("out,o", po::value<string>(&out_f), "Output file")
		("no-header,n", po::value<bool>(&no_header)->default_value(false)->zero_tokens(), "Do not print column identifier")
		("round,r", po::value<short>(&precision)->default_value(2), "The number of digits to print after decimal point")
	;

	po::options_description table_opts("Input format options");
	table_opts.add_options()
		("delimiter,d", po::value<char>(&delim)->default_value('\t', "TAB"), "Delimiter to use")
		("merge,m", po::value<bool>(&merge)->default_value(false)->zero_tokens(), "Merge several separators into one")
		("columns,c", po::value<string>(&col_str)->required(), "The columns to use")
		("grouping,k", po::value<int>(&k)->default_value(1), "Every k-th row will be grouped together")
		("patters,p", po::value<vector<string> >(&patterns)->multitoken(), "Pattern to use for distinguishing values")
		("skip_lines,l", po::value<int>(&skip_lines)->default_value(0), "Skips the first n lines")
	;


	po::options_description output_opts("Algorithmic options:");
	output_opts.add_options()
		("average,a", po::value<bool>(&average)->default_value(false)->zero_tokens(), "Calculates the average")
		("max", po::value<bool>(&max_value)->default_value(false)->zero_tokens(), "Calculates the sum")
		("min", po::value<bool>(&min_value)->default_value(false)->zero_tokens(), "Calculates the minimum")
	//	("sum,s", po::value<bool>(&sum)->default_value(false)->zero_tokens(), "Calculates the maximum")
	;

	po::options_description all("colsum (" + version + ") Copyright (C) 2014  Carsten Kemena\nThis program comes with ABSOLUTELY NO WARRANTY;\n\nAllowed options are displayed below.");
	all.add(general_opts).add(table_opts).add(output_opts);
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


	int n_options=min_value + max_value + average;
	if (n_options > 1)
	{
		cerr << "Error please specify at most one of average,min,max" << endl;
		return EXIT_SUCCESS;
	}
	else if (n_options ==0 )
		sum=true;

	if ((k>1) && (!patterns.empty()))
	{
		cerr << "Error! patterns and grouping options cannot be used at the same time!\n" << endl;
		return EXIT_SUCCESS;
	}
	vector<int> col_ids;
	str2col_id(col_str, col_ids);
	if (col_ids.empty())
	{
		cerr << "Error! Please check column string\n" << endl;
		return EXIT_FAILURE;
	}

	double (*func2use)(double, double) = NULL;
	double init_val;
	if (sum || average)
	{
		func2use=my_sum;
		init_val = 0;
	}
	if (max_value)
	{
		func2use=my_max;
		init_val = (-1)*std::numeric_limits<double>::max();
	}
	if (min_value)
	{
		func2use=my_min;
		init_val = std::numeric_limits<double>::max();
	}

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


	// begin calculation
	string line;

	size_t i;
	size_t n_cols = col_ids.size();
	if (!patterns.empty())
		k=patterns.size();
	double **values = new double*[k];
	int *line_counter = new int[k];
	for (int j=0; j<k; ++j)
	{
		line_counter[j] = 0;
		values[j] = new double[n_cols];
		for (i=0; i<n_cols; ++i)
			values[j][i]=init_val;
	}
	int x=0;
	int line_n=0;

	// skip the specified number of lines at the start of the file
	int skip=-1;
	while(!in_p->eof() && (++skip<skip_lines))
		getline(*in_p, line);

	bool empty_line_found=false;
	try
	{
		if (patterns.empty())
		{
			// sum value according to k-th row
			while(getline(*in_p, line))
			{
				if (empty_line_found)
				{
					parse_line("", delim, col_ids, values[x], line_n, merge, func2use);
				}
				if (!line.empty())
				{
					++line_n;
					parse_line(line, delim, col_ids, values[x], line_n, merge, func2use);
					++line_counter[x];
					if (x+1==k)
						x=0;
					else
						++x;
				}
				else
				{
					empty_line_found=true;
				}
			}
		}
		else
		{
			vector<boost::regex> expressions;
			expressions.resize(k);
			for (int pattern_id=0; pattern_id<k; ++pattern_id)
				expressions[pattern_id].assign(patterns[pattern_id]);
			boost::cmatch what;

			// sum values according to pattern
			while(!in_p->eof())
			{
				getline(*in_p, line);
				++line_n;
				for (x=0; x<k; ++x)
				{
					if(boost::regex_search(line.c_str(), what, expressions[x]))
					{
						parse_line(line, delim, col_ids, values[x], line_n, merge, func2use);
						++line_counter[x];
					}
				}
			}
		}
	}
	catch(std::exception &e)
	{
	    cerr << "Error in line " << line_n << "! Cannot cast value." << endl;
	    exit(EXIT_FAILURE);
	}


	// printing the output
	if (!no_header) // printing header
	{
		if (!patterns.empty())
			(*out_p)<< "group ";
		(*out_p) << (col_ids[0]+1);
		for (i=1; i<col_ids.size(); ++i)
			(*out_p) << " " <<  col_ids[i]+1;
		(*out_p) << endl;
	}

	cout.precision(precision);
	for (int j=0; j<k; ++j) // print the results
	{
		if (!patterns.empty())
			(*out_p) << patterns[j].c_str() << " ";
		if (average)
		{
			(*out_p) <<fixed<< (values[j][0]/line_counter[j]);
			for (i=1; i<col_ids.size(); ++i)
				(*out_p) <<fixed<< " " << (values[j][i]/line_counter[j]);
		}
		else
		{
			(*out_p) <<fixed<<  values[j][0];
			for (i=1; i<col_ids.size(); ++i)
				(*out_p) <<fixed<< " "<< values[j][i];
		}
		(*out_p) << endl;
	}

	// close files & release memory
	if (!in_f.empty())
		in_F.close();
	if (!out_f.empty())
		out_F.close();

	for (int j=0; j<k; ++j)
		delete[] values[j];

	delete[] values;
	delete[] line_counter;
	exit(EXIT_SUCCESS);

}

