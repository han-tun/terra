// Copyright (c) 2018-2021  Robert J. Hijmans
//
// This file is part of the "spat" library.
//
// spat is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// spat is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with spat. If not, see <http://www.gnu.org/licenses/>.

#include "spatDataframe.h"
#include <vector>
#include <algorithm>
#include <string>
#include "NA.h"
#include "string_utils.h"


SpatDataFrame::SpatDataFrame() {}

SpatDataFrame SpatDataFrame::skeleton() {
	SpatDataFrame out;
	out.names  = names;
	out.itype  = itype;
	out.iplace = iplace;
	out.dv = std::vector<std::vector<double>>(dv.size());
	out.iv = std::vector<std::vector<long>>(iv.size());
	out.sv = std::vector<std::vector<std::string>>(sv.size());
	return out;
}


std::vector<double> SpatDataFrame::getD(unsigned i) {
	unsigned j = iplace[i];
	return dv[j];
}

double SpatDataFrame::getDvalue(unsigned i, unsigned j) {
	j = iplace[j];
	return dv[j][i];
}

std::vector<long> SpatDataFrame::getI(unsigned i) {
	unsigned j = iplace[i];
	return iv[j];
}


long SpatDataFrame::getIvalue(unsigned i, unsigned j) {
	j = iplace[j];
	return iv[j][i];
}


std::vector<std::string> SpatDataFrame::getS(unsigned i) {
	unsigned j = iplace[i];
	return sv[j];
}


std::string SpatDataFrame::getSvalue(unsigned i, unsigned j) {
	j = iplace[j];
	return sv[j][i];
}


SpatDataFrame SpatDataFrame::subset_rows(unsigned i) {
	std::vector<unsigned> r = { i }; 
	return subset_rows(r);
}


SpatDataFrame SpatDataFrame::subset_rows(std::vector<unsigned> range) { 

	SpatDataFrame out;

	unsigned nr = nrow();
	for (int i = range.size(); i>0; i--) {
		if (range[i-1] > nr) {
			range.erase(range.begin() + i-1);
		}
	}

	out.names = names;
	out.itype = itype;
	out.iplace = iplace;

	out.dv.resize(dv.size());
	out.iv.resize(iv.size());
	out.sv.resize(sv.size());

	out.dv.reserve(range.size());
	out.iv.reserve(range.size());
	out.sv.reserve(range.size());

	for (size_t i=0; i < range.size(); i++) {
		for (size_t j=0; j < dv.size(); j++) {
			out.dv[j].push_back(dv[j][range[i]]);
		}
		for (size_t j=0; j < iv.size(); j++) {
			out.iv[j].push_back(iv[j][range[i]]);
		}
		for (size_t j=0; j < sv.size(); j++) {
			out.sv[j].push_back(sv[j][range[i]]);
		}
	}
	return out;
}


SpatDataFrame SpatDataFrame::subset_cols(unsigned i) {
	std::vector<unsigned> c = { i }; 
	return subset_cols(c);
}

SpatDataFrame SpatDataFrame::subset_cols(std::vector<unsigned> range) { 
	SpatDataFrame out;
	unsigned dcnt=0;
	unsigned icnt=0;
	unsigned scnt=0;
	for (size_t i=0; i < range.size(); i++) {
		if (range[i] < 0 || range[i] >= ncol()) {
			out.setError("invalid column");
			return out;
		}
		unsigned j = range[i];
		unsigned p = iplace[j];
		out.names.push_back(names[j]);
		if (itype[j] == 0) {
			out.dv.push_back(dv[p]);
			out.iplace.push_back(dcnt);
			out.itype.push_back(0);
			dcnt++;
		} else if (itype[j] == 1) {
			out.iv.push_back(iv[p]);
			out.iplace.push_back(icnt);
			out.itype.push_back(1);
			icnt++;			
		} else {
			out.sv.push_back(sv[p]);
			out.iplace.push_back(scnt);
			out.itype.push_back(2);
			scnt++;			
		}
	}
	return out;
}


unsigned SpatDataFrame::ncol() {
	return itype.size();
}


unsigned SpatDataFrame::nrow() {
	unsigned n;
	if (itype.size() == 0) {
		n = 0;
	} else {
		if (itype[0] == 0) {
			n = dv[0].size();
		} else if (itype[0] == 1) {
			n = iv[0].size();
		} else {
			n = sv[0].size();		
		}
	}
	return n;
}




void SpatDataFrame::add_row() {
	for (size_t i=0; i < dv.size(); i++) {
		dv[i].push_back(NAN);
	}
	long longNA = NA<long>::value;
	for (size_t i=0; i < iv.size(); i++) {
		iv[i].push_back(longNA);
	}
	for (size_t i=0; i < sv.size(); i++) {
		sv[i].push_back(NAS);
	}
}


void SpatDataFrame::reserve(unsigned n) {
	for (size_t i=0; i<dv.size(); i++) {
		dv[i].reserve(n);
	}
	for (size_t i=0; i<iv.size(); i++) {
		iv[i].reserve(n);
	}
	for (size_t i=0; i<sv.size(); i++) {
		sv[i].reserve(n);
	}
}

void SpatDataFrame::resize_rows(unsigned n) {
	for (size_t i=0; i<dv.size(); i++) {
		dv[i].resize(n, NAN);
	}
	long longNA = NA<long>::value;
	for (size_t i=0; i<iv.size(); i++) {
		iv[i].resize(n, longNA);
	}
	for (size_t i=0; i<sv.size(); i++) {
		sv[i].resize(n, NAS);
	}
}

void SpatDataFrame::resize_cols(unsigned n) {
	if (n < ncol()) {
		itype.resize(n);
		iplace.resize(n);
	} else {
		setError("you can only resize to fewer columns");
	}
}

// use template instead
bool SpatDataFrame::add_column(std::vector<double> x, std::string name) {
	unsigned nr = nrow();
	if ((nr != 0) & (nr != x.size())) return false; 
	iplace.push_back(dv.size());
	itype.push_back(0);
	names.push_back(name);
	dv.push_back(x);
	return true;
}


bool SpatDataFrame::remove_column(int i) {

	if ((i < 0) | ((size_t)i > ncol())) {
		return false;
	}
	size_t dtype = itype[i];
	size_t place = iplace[i];

	for (size_t i=(place+1); i<iplace.size(); i++) {
		if (itype[i] == dtype) {
			iplace[i]--;
		}
	}

	names.erase(names.begin()+i);
	itype.erase(itype.begin()+i);
	iplace.erase(iplace.begin()+i);
	if (dtype == 0) {
		dv.erase(dv.begin()+place);
	} else if (dtype == 1) {
		iv.erase(iv.begin()+place);
	} else {
		sv.erase(sv.begin()+place);
	}
	return true;
}

bool SpatDataFrame::remove_column(std::string field) {
	int i = where_in_vector(field, names, false);
	return remove_column(i);
}



bool SpatDataFrame::add_column(std::vector<long> x, std::string name) {
	unsigned nr = nrow();
	if ((nr != 0) & (nr != x.size())) return false; 
	iplace.push_back(iv.size());
	itype.push_back(1);
	names.push_back(name);
	iv.push_back(x);
	return true;
}

bool SpatDataFrame::add_column(std::vector<int> x, std::string name) {
	std::vector<long> v(x.begin(), x.end());
	return add_column(v, name);
}


bool SpatDataFrame::add_column(std::vector<std::string> x, std::string name) {
	unsigned nr = nrow();
	if ((nr != 0) & (nr != x.size())) return false; 
	iplace.push_back(sv.size());
	itype.push_back(2);
	names.push_back(name);
	sv.push_back(x);
	return true;
}


void SpatDataFrame::add_column(unsigned dtype, std::string name) {
	unsigned nr = nrow();
	if (dtype == 0) {
		std::vector<double> dins(nr, NAN);
		iplace.push_back(dv.size());
		dv.push_back(dins);
	} else if (dtype == 1) {
		long longNA = NA<long>::value;
		std::vector<long> iins(nr, longNA);
		iplace.push_back(iv.size());
		iv.push_back(iins);
	} else {
		std::vector<std::string> sins(nr, NAS);
		iplace.push_back(sv.size());
		sv.push_back(sins);
	}
	itype.push_back(dtype);
	names.push_back(name);
}

bool SpatDataFrame::cbind(SpatDataFrame &x) {
	unsigned nc = x.ncol();
	std::vector<std::string> nms = x.names;
	for (size_t i=0; i<nc; i++) {
		if (x.itype[i] == 0) {
			std::vector<double> d = x.getD(i);
			if (!add_column(d, nms[i])) return false;
		} else if (x.itype[i] == 1) {
			std::vector<long> d = x.getI(i);
			if (!add_column(d, nms[i])) return false;
		} else {
			std::vector<std::string> d = x.getS(i);
			if (!add_column(d, nms[i])) return false;
		}
	}
	return true;
}


bool SpatDataFrame::rbind(SpatDataFrame &x) {

	size_t nr1 = nrow();
	size_t nr2 = x.nrow();
//	size_t nc1 = ncol();
	size_t nc2 = x.ncol();

//first add new columns
	std::vector<std::string> nms = names;
	for (size_t i=0; i<nc2; i++) {
		int j = where_in_vector(x.names[i], nms, false);
		if (j < 0) { // not in df
			size_t b = x.iplace[i];
			add_column(x.itype[i], x.names[i]);
			if (x.itype[i] == 0) { 
				size_t a = dv.size()-1;
				dv[a].insert(dv[a].begin()+nr1, 
					x.dv[b].begin(), x.dv[b].end());
			} else if (x.itype[i] == 1) { 
				size_t a = iv.size()-1;
				iv[a].insert(iv[a].begin()+nr1, 
					x.iv[b].begin(), x.iv[b].end());
			} else {
				size_t a = sv.size()-1;
				sv[a].insert(sv[a].begin()+nr1, 
					x.sv[b].begin(), x.sv[b].end());
			} 
		} else {
			size_t a = iplace[j];
			size_t b = x.iplace[i];
			if (itype[j] == x.itype[i]) {
				if (itype[j] == 0) { 
					dv[a].insert(dv[a].begin()+nr1, 
						x.dv[b].begin(), x.dv[b].end());
				} else if (itype[j] == 1) { 
					iv[a].insert(iv[a].begin()+nr1, 
						x.iv[b].begin(), x.iv[b].end());
				} else {
					sv[a].insert(sv[a].begin()+nr1, 
						x.sv[b].begin(), x.sv[b].end());
				} 
			} else {
				if (itype[j] == 2) {
					sv[a].resize(nr1);
					if (x.itype[i] == 0) {
						for (size_t k=0; k<nr2; k++) {
							sv[a].push_back(std::to_string(x.dv[b][k]));
						}
					} else {
						for (size_t k=0; k<nr2; k++) {
							sv[a].push_back(std::to_string(x.iv[b][k]));
						}					
					}
				} else if (itype[j] == 0) { 
					if (x.itype[i] == 1) {
						dv[a].resize(nr1);
						for (size_t k=0; k<nr2; k++) {
							dv[a].push_back(x.iv[b][k]);
						}
					}
				} else if (itype[j] == 1) { 
					if (x.itype[i] == 0) {
						iv[a].resize(nr1);
						for (size_t k=0; k<nr2; k++) {
							iv[a].push_back(x.dv[b][k]);
						}
					}
				}
				//degrade types
			}
		}
	}
	
	resize_rows(nr1 + nr2);
	return true;
}


std::vector<std::string> SpatDataFrame::get_names() {
	return names;
}


void SpatDataFrame::set_names(std::vector<std::string> nms){
	if (ncol() == nms.size()) {
        make_valid_names(nms);
        make_unique_names(nms);
		names = nms;
	} else {
		setError("number of names is not correct");
	}
}


std::vector<std::string> SpatDataFrame::get_datatypes() {
	std::vector<std::string> types = {"double", "long", "string"};
	std::vector<std::string> stype(itype.size());
	for (size_t i=0; i<itype.size(); i++) {
		stype[i] = types[itype[i]]; 
	}
	return stype;
}


std::string SpatDataFrame::get_datatype(std::string field) {
	int i = where_in_vector(field, get_names(), false);
	if (i < 0) return "";
	i = itype[i]; 
	std::vector<std::string> types = {"double", "long", "string"};
	return types[i]; 
}

std::string SpatDataFrame::get_datatype(int field) {
	if ((field < 0) || (field > (int)(ncol()-1))) return "";
	std::vector<std::string> types = {"double", "long", "string"};
	return types[itype[field]]; 
}

bool SpatDataFrame::field_exists(std::string field) {
	return is_in_vector(field, get_names()); 
}


int SpatDataFrame::get_fieldindex(std::string field) {
	return where_in_vector(field, get_names(), false);
}


// only doing this for one column for now
SpatDataFrame SpatDataFrame::unique(int col) {
	SpatDataFrame out = subset_cols(col);
	
	if (out.hasError()) return out;
	if (out.itype[0] == 0) {
		size_t sz = nrow();
		out.dv[0].erase(std::remove_if(out.dv[0].begin(), out.dv[0].end(),
            [](const double& value) { return std::isnan(value); }), out.dv[0].end());
		bool hasNAN = sz > out.dv[0].size();
		std::sort(out.dv[0].begin(), out.dv[0].end());
		out.dv[0].erase(std::unique(out.dv[0].begin(), out.dv[0].end()), out.dv[0].end());
		if (hasNAN) out.dv[0].push_back(NAN);
		
	} else if (out.itype[0] == 1) {
		std::sort(out.iv[0].begin(), out.iv[0].end());
		out.iv[0].erase(std::unique(out.iv[0].begin(), out.iv[0].end()), out.iv[0].end());
	} else {
		std::sort(out.sv[0].begin(), out.sv[0].end());
		out.sv[0].erase(std::unique(out.sv[0].begin(), out.sv[0].end()), out.sv[0].end());
	}
	return out;
}


std::vector<int> SpatDataFrame::getIndex(int col, SpatDataFrame &x) {
	size_t nd = nrow();
	x = unique(col);
	size_t nu = x.nrow();
	std::vector<int> idx(nd, -1);
	size_t ccol = iplace[col];
	if (x.itype[0] == 0) {
		for (size_t i=0; i<nd; i++) {
			for (size_t j=0; j<nu; j++) {
				if (std::isnan(dv[ccol][i]) && (std::isnan(x.dv[0][j]))) {
					idx[i] = j;
					continue;					
				} else if (dv[ccol][i] == x.dv[0][j]) {
					idx[i] = j;
					continue;
				}
			}
		}
	} else if (x.itype[0] == 1) {
		for (size_t i=0; i<nd; i++) {
			for (size_t j=0; j<nu; j++) {
				if (iv[ccol][i] == x.iv[0][j]) {
					idx[i] = j;
					continue;
				}
			}
		}
	} else {
		for (size_t i=0; i<nd; i++) {
			for (size_t j=0; j<nu; j++) {
				if (sv[ccol][i] == x.sv[0][j]) {
					idx[i] = j;
					continue;
				}
			}
		}
	}
	return idx;
}

