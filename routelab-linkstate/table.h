#ifndef _table
#define _table

#include <iostream>
#include <vector>
#include <map>
#include "node.h"
#include "link.h"

#if defined(GENERIC)

		class Table {
		 public:
		  ostream & Print(ostream &os) const;
		};

#endif


#if defined(LINKSTATE)

		class TableItem {
		public:
		  int cost;
		  int age;
		  TableItem() { 
		  	cost = -1; 
		  	age = 0;
		  };
		  TableItem(int c, int a) { 
		  	cost = c; 
		  	age = a; 
		  };
		};

		class Table {
		public:
		  map<int, map<int, TableItem> > table;
		  Table() {};
		  bool UpdateTable(const RoutingMessage * m);
		  int UpdateLink(const Link *l);
		  void PrintTable();
		  ostream & Print(ostream &os) const;
		};

#endif


#if defined(DISTANCEVECTOR)

		#include <deque>

				class Node;
				class Link;

				class Table {
				private: 
				  vector<vector<double> > table;
				public:
				  Table();
				  Table(Node me);
				  bool CheckLatency(int s, int n, double d);
				  bool WriteTable(int s, int n, double d);
				  double ReadTable(int s, int n);
				  int GetNext(int d);
				  ostream & Print(ostream &os) const;
				};

#endif

inline ostream & operator<<(ostream &os, const Table &t) { return t.Print(os);}


#endif