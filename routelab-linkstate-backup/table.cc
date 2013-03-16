#include "table.h"
#include "messages.h"

#if defined(GENERIC)

ostream & Table::Print(ostream &os) const { os << "Table()"; return os; }

#endif

#if defined(LINKSTATE)

		int Table::UpdateLink(const Link *link)
		{
			int source = link->GetSrc();
			int destination = link->GetDest();
			int latency = link->GetLatency();
			int age = ++table[source][destination].age;

			table[source][destination].age = age;
		  table[source][destination].cost = latency;
		  return age;
		}

		bool Table::UpdateTable(const RoutingMessage * message)
		{
			int source = message->source;
			int destination = message->destination;
			int age = message->age;
			int latency = message->latency;

		  if ((table[source][destination].cost == -1) || (table[source][destination].age < age))
		  {
		    table[source][destination].cost = latency;
		    table[source][destination].age = age;
		    return true;
		  }
		  else {
				return false;
		  }

		}

		void Table::PrintTable()
		{
			cout << "----------------------------" << endl;  

		   for(map<int, map<int,TableItem> >::const_iterator i = table.begin(); i != table.end(); ++i)
		   {
		      cout << i->first << ": ";
		       for(map<int, TableItem >::const_iterator ii = (i)->second.begin(); ii != (i)->second.end(); ++ii)
		       {
		         cout << ii->first << "(" << ii->second.cost << "), ";
		       }
		       cout << endl;
		    }

		  cout << "----------------------------" << endl;  
		}


#endif

#if defined(DISTANCEVECTOR)

		Table::Table() {}

		Table::Table(Node n) 
		{

		  deque<Link*> *links = n.GetOutgoingLinks();
		  for (deque<Link*>::iterator i = links->begin(); i != links->end(); ++i)
		  {
		  	int destination = (*i)->GetDest();
		  	int latency = (*i)->GetLatency();

		  	WriteTable(destination, destination, latency);
		  }
		     
		  delete links;

		}

		bool
		Table::CheckLatency(int destination, int next, double distance) 
		{

		  if (ReadTable(destination, next) > distance || ReadTable(destination, next) == -1)
		  {
		  	return 1;
		  }
		  else {
		  	return 0;
		  }

		}

		bool
		Table::WriteTable(int destination, int next, double distance) 
		{

		  while (table.size() <= destination) 
		  {
		  	table.push_back(*new(vector<double>));
		  }

		  while (table[destination].size() <= next) 
		  {
		  	table[destination].push_back((double) -1);
		  }

		  table[destination][next] = distance;

		  return CheckLatency(destination, next, distance);;

		}

		double
		Table::ReadTable(int destination, int next)
		{
		  if (table.size() <= destination || table[destination].size() <= next)
		  {
		  	return -1;
		  }
		  else 
			{
				return table[destination][next];
			}
		}

		int
		Table::GetNext(int destination)
		{
		  if (table.size() <= destination)
		  {
		  	return -1;
		  }

		  int ret = 0;
		  int counter = 0;
		  for (vector<double>::const_iterator i = table[destination].begin(); i != table[destination].end(); ++i)
		  {
		    if ((*i) != -1 && ((*i) < table[destination][ret] || table[destination][ret] == -1))
		    {
					ret = counter;
		    }        

		    counter++;
		  }
		  return ret;
		}

		ostream&
		Table::Print(ostream &os) const 
		{
		  if (this == NULL) 
		  {
		  	return os;
		  }

		  os << endl << "Table: " << endl;
		  int counter = 0;
		  for (vector<vector<double> >::const_iterator i = table.begin(); i != table.end(); ++i)
		  {
		    os << "to " << counter++ << ": ";
		    for (vector<double>::const_iterator j = (*i).begin(); j != (*i).end(); ++j) 
		    {
		        os << "\t" << (*j);
		    }
		    os << endl;
		  }
		  return os;
		}

#endif