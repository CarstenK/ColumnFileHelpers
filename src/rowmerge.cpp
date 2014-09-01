/*
 * rowmerge.cpp
 *
 *  Created on: Jun 18, 2013
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


// boost headers
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

using boost::lexical_cast;
using boost::bad_lexical_cast;
namespace po = boost::program_options;

#include "Version.hpp"

void
parse_line(const string &line, char delim, int col_id, size_t &start, size_t &end, size_t n_line, bool merge, string in_f)
{
	start=0;
	size_t len = line.length();
	int curr_col=1;
	size_t line_pos=0;
	const char *line_c=line.c_str();
	bool found=false;
	while (line_pos<len)
	{
		start=line_pos;
		while ((line_pos != len) && (line[line_pos] != delim))
			++line_pos;
		if (curr_col==col_id)
		{
			found=true;
			end=line_pos;
			break;
		}
		++line_pos;
		if (merge)
		{
			while ((line_pos != len) && (line_c[line_pos] == delim))
				++line_pos;
		}
		++curr_col;
	}

	if (!found)
	{
		fprintf(stderr, "Error! Line %li in %s does not contain enough columns!\n", n_line, in_f.c_str());
		exit(EXIT_FAILURE);
	}
}


int
main(int argc, char *argv[])
{
	vector<string> in_f;
	string out_f;
	vector<int> cols;
	char delim;
	bool merge;
	po::options_description general_opts("General options");
	general_opts.add_options()
		("help,h", "Produces this help message")
		("in,i", po::value<vector<string> >(&in_f)->multitoken(), "Input file")
		("out,o", po::value<string>(&out_f), "Output file")
		("col,c", po::value<vector<int> >(&cols)->multitoken(), "Columns in file")
		("delimiter,d", po::value<char>(&delim)->default_value('\t'), "Delimiter")
		("merge,m", po::value<bool>(&merge)->default_value(false), "Consecutive delimiters are treated as one")
	;

	po::options_description all("rowmerge (" + version + ") Copyright (C) 2014  Carsten Kemena\nThis program comes with ABSOLUTELY NO WARRANTY;\n\nAllowed options are displayed below.");
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


	if (in_f.size() < 2)
	{
		cerr << "Error! At least two input files need to be given!" << endl;
		return EXIT_SUCCESS;
	}

	ifstream in_F;
	string col_id;
	std::string line;

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

	string dels="";
	dels.push_back(delim);
	int col=-100;
	string col_name;
	bool automatic = false;
	boost::algorithm::token_compress_mode_type compress_mode;
	if (merge)
		compress_mode=boost::algorithm::token_compress_on;
	else
		compress_mode=boost::algorithm::token_compress_off;
	if (cols.empty())
	{
		automatic=true;
		in_F.open(in_f[0].c_str(), ifstream::in);
		if (!in_F.good())
		{
			cerr << "Error! Could not open file " << in_f[0] << endl;
			exit(EXIT_FAILURE);
		}
		vector<string> file_headers1, file_headers2;
		getline(in_F, line);
		boost::split(file_headers1,line,boost::is_any_of(dels), compress_mode);
		in_F.close();


		in_F.open(in_f[1].c_str(), ifstream::in);
		if (!in_F.good())
		{
			cerr << "Error! Could not open file " << in_f[1] << endl;
			exit(EXIT_FAILURE);
		}
		getline(in_F, line);
		boost::split(file_headers2,line,boost::is_any_of(dels), compress_mode);
		in_F.close();
		size_t n_columns2 = file_headers2.size();
		size_t n_columns1 = file_headers1.size();
		size_t i,j;
		for (i=0; i<n_columns1; ++i)
		{
			for (j=0; j<n_columns2; ++j)
			{
				if (file_headers1[i]==file_headers2[j])
					col=i;
			}
		}
		col_name=file_headers1[col];
		++col;
		cout << col_name << endl;
		in_F.close();
	}
	else
		col = cols[0];

	map<string, pair<size_t,string> > lines;
	size_t start=-1, end=-1;
	size_t n_line=0;

	in_F.open(in_f[0].c_str(), ifstream::in);
	if (!in_F.good())
	{
		cerr << "Error! Could not open file " << in_f[0] << endl;
		exit(EXIT_FAILURE);
	}


	while (getline(in_F, line))
	{
		++n_line;
		if (!line.empty())
		{
			parse_line(line, delim, col, start, end, n_line, merge, in_f[0]);
			lines[line.substr(start, end-start)] = pair<size_t, string>(1, line);
		}
	}
	in_F.close();

	n_line=0;
	string key;
	typedef map<string, pair<size_t,string> >::const_iterator MapIterator;
	MapIterator it, it_end=lines.end();
	size_t i,j;
	vector<string> file_headers;
	size_t n_columns;
	for (i=1; i<in_f.size(); ++i)
	{
		in_F.open(in_f[i].c_str(), ifstream::in);
		if (!in_F.good())
		{
			cerr << "Error! Could not open file " << in_f[i] << endl;
			exit(EXIT_FAILURE);
		}


		if (automatic)
		{
			getline(in_F, line);
			boost::split(file_headers,line,boost::is_any_of(dels), compress_mode);

			n_columns=file_headers.size();
			for (j=0; j<n_columns; ++j)
			{
				if (file_headers[j] == col_name)
				{
					cout << file_headers[j] << " " <<j <<endl;
					col=j+1;
					break;
				}
			}
			cout << col << endl;
			in_F.seekg (0, in_F.beg);

		}
		else
			col= (cols.size()==1) ? cols[0] : cols[i];
		while (getline(in_F, line))
		{
			if (!line.empty())
			{
				parse_line(line, delim, col, start, end, n_line, merge, in_f[i]);
				key=line.substr(start, end-start);
				if (lines.find(key) != it_end)
				{
					++lines[key].first;
					lines[key].second.push_back(delim);
					lines[key].second.append(line.substr(0, start));
					if (line.size()-end > 0)
						lines[key].second.append(line.substr(end+1, line.size()-end));
				}

			}
		}
		in_F.close();
	}


	for (MapIterator iter = lines.begin(); iter != lines.end(); iter++)
	{
		if (iter->second.first==in_f.size())
			(*out_p) << iter->second.second << endl;
	}


	if (!out_f.empty())
		out_F.close();

}


