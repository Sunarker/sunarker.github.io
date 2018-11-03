#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>
#include <iomanip>
#include <cmath>

using namespace std;

map<string,int> vocabindex;
map<int,string> indexvocab;
int topicn = 0;
int usern = 0;
int resourcen = 0;

void getVocabulary(char*filepath)
{
	ifstream f1(filepath);
	string temp;
	while(getline(f1,temp))
	{
		istringstream instr(temp);
		string word;
		while(instr>>word)
		{
			map<string,int>::iterator iter1;
			iter1 = vocabindex.find(word);
			if(iter1 == vocabindex.end())
				vocabindex.insert(pair<string,int>(word,vocabindex.size()));
		}
	}
	f1.close();

	ofstream f2("vocabindex.txt");
	map<string,int>::iterator iter2;
	for(iter2 = vocabindex.begin();iter2 != vocabindex.end();iter2++)
	{
		f2<<iter2 -> first<<" "<<iter2 -> second<<endl;
		indexvocab.insert(pair<int,string>(iter2->second,iter2->first));
	}
	f2.close();

	cout<<"vocabulary size is "<<vocabindex.size()<<endl;
}

void indexContents(char*filepath)
{
	ifstream f1(filepath);
	ofstream f2("filesnumeric.txt");
	string temp;
	while(getline(f1,temp))
	{
		istringstream instr(temp);	
		string word;
		while(instr>>word)
		{
			f2<<vocabindex[word]<<" ";
		}
		f2<<endl;
	}
	f1.close();
	f2.close();
	cout<<"index contents ok"<<endl;
}

void getUsersResources(char *filepath)
{
	vector<int>users;
	vector<int>resources;

	ifstream f(filepath);
	string temp;
	while(getline(f,temp))
	{
		int user,resource;
		istringstream instr(temp);
		while(instr>>user && instr>>resource)
		{
			users.push_back(user);
			resources.push_back(resource);
		}
	}
	f.close();

	sort(users.begin(),users.end());
	sort(resources.begin(),resources.end());
	vector<int>::iterator u_end = unique(users.begin(),users.end());
	vector<int>::iterator r_end = unique(resources.begin(),resources.end());
	users.erase(u_end,users.end());
	resources.erase(r_end,resources.end());

	usern = users.size();
	resourcen = resources.size();

	cout<<"users number is "<<usern<<endl;
	cout<<"resources number is "<<resourcen<<endl;
}

void initialTopics()
{
	ifstream f1("filesnumeric.txt");
	ofstream f2("topics1.txt");
	string temp;
	while(getline(f1,temp))
	{
		istringstream instr(temp);
		int word;
		while(instr>>word)
		{
			int topic = (int)(((double)random()/RAND_MAX)*topicn);
			f2<<topic<<" ";
		}
		f2<<endl;
	}
	f1.close();
	f2.close();
}

void initialParams(char*fileidpath,double alpha1[],double alpha2[],double beta[],int **uxk,int **rxk,int **vxk, double denominator[])
{
	for(int i=0;i<topicn;i++)
	{
		alpha1[i] = 0.1;
		alpha2[i] = 0.1;
	}
	for(int i=0;i<vocabindex.size();i++)
	{
		beta[i] = 0.01;
	}

	for(int i=0;i<topicn;i++)
	{
		for(int j=0;j<usern;j++)
			uxk[j][i] = 0;
		for(int j=0;j<resourcen;j++)
			rxk[j][i] = 0;
		for(int j=0;j<vocabindex.size();j++)
			vxk[j][i] = 0;
	}

	ifstream f1(fileidpath);
	ifstream f2("filesnumeric.txt");
	ifstream f3("topics1.txt");
	string temp1,temp2,temp3;
	while(getline(f1,temp1) && getline(f2,temp2) && getline(f3,temp3))
	{
		istringstream instr1(temp1);
		istringstream instr2(temp2);
		istringstream instr3(temp3);

		int user,resource;
		instr1>>user;
		instr1>>resource;

		int wordind,topic;
		while(instr2>>wordind && instr3>>topic)
		{
			uxk[user][topic]++;
			rxk[resource][topic]++;
			vxk[wordind][topic]++;
		}
	}	
	f1.close();
	f2.close();
	f3.close();

	for(int i=0;i<topicn;i++)
	{
		denominator[i] = 0;
		for(int j=0;j<vocabindex.size();j++)
		{
			denominator[i] = denominator[i] + beta[i] + vxk[j][i];
		}
		//cout<<"denominator[i]"<<denominator[i]<<endl;
		//if(denominator[i] <= 0.1*vocabindex.size())
			//cout<<"denominator error"<<endl;
	}
}

void testError(double part1,double part2,double part3,double part4)
{
	if(part1<0)
		cout<<"part1 error"<<endl;
	if(part2<0)
		cout<<"part2 error"<<endl;
	if(part3<0)
		cout<<"part3 error"<<endl;
	if(part4<=0)
		cout<<"part4 error"<<endl;
}

void testP(double p[])
{
	for(int i=0;i<topicn;i++)
	{
		cout<<p[i]<<" ";	
	}
	cout<<endl;
}

void jlda(char*fileidpath,double alpha1[],double alpha2[],double beta[],int **uxk,int **rxk,int **vxk,double denominator[],int iters)
{
	ofstream jf("jlikelihood.txt");
	for(int iter=0;iter<iters;iter++)
	{
		cout<<"iter: "<<iter<<endl;

		double likelihood = 0;

		ifstream f1(fileidpath);
		ifstream f2("filesnumeric.txt");
		ifstream f3;
		ofstream f4;
		
		if(iter%2 == 0)
		{
			f3.open("topics1.txt");
			f4.open("topics2.txt");
		}
		else
		{
			f3.open("topics2.txt");
			f4.open("topics1.txt");
		}

		string temp1,temp2,temp3;
		while(getline(f1,temp1) && getline(f2,temp2) && getline(f3,temp3))
		{
			istringstream instr1(temp1);
			istringstream instr2(temp2);
			istringstream instr3(temp3);
			
			int user,resource;
			instr1>>user;
			instr1>>resource;
			
			int wordind,topic;
			while(instr2>>wordind && instr3>>topic)
			{
				uxk[user][topic]--;
				rxk[resource][topic]--;
				vxk[wordind][topic]--;
				denominator[topic]--;
				
				double p[topicn];
				double sump=0;
				for(int i=0;i<topicn;i++)
				{
					double part1 = alpha1[i] + uxk[user][i];
					double part2 = alpha2[i] + rxk[resource][i];
					double part3 = beta[wordind] + vxk[wordind][i];
					double part4 = denominator[i];
					testError(part1,part2,part3,part4);

					p[i] = sqrt(part1)*sqrt(part2)*part3/part4;
					sump = sump + p[i];
				}

				double cdf[topicn];
				for(int i=0;i<topicn;i++)
				{
					if(i==0)
						cdf[i] = p[i];
					else
						cdf[i] = cdf[i-1] + p[i];
				}
				
				//testP(p);				

				double randp = ((double)random()/RAND_MAX)*cdf[topicn-1];	
				//cout<<"randp:"<<randp<<endl;
				int newtopic;			
				for(int i=0;i<topicn;i++)
				{
					if(randp<=cdf[i])
					{
						newtopic = i;
						likelihood = likelihood + log(p[i]/sump);
						break;
					}
				}
				
				//cout<<"newtopic:"<<newtopic<<endl;
				uxk[user][newtopic]++;
				rxk[resource][newtopic]++;
				vxk[wordind][newtopic]++;
				denominator[newtopic] = denominator[newtopic] + 1;
				f4<<newtopic<<" ";
			}
			f4<<endl;
		}
		
		f1.close();
		f2.close();
		f3.close();
		f4.close();
		jf<<likelihood<<endl;
	}
	jf.close();
}

void saveres(double alpha1[],double alpha2[],double beta[],int **uxk,int **rxk,int **vxk,int iters)
{

	ofstream f1("utopics.txt");
	for(int i=0;i<usern;i++)
	{
		double summer = 0;
		for(int j=0;j<topicn;j++)
		{
			summer = summer + alpha1[j] + uxk[i][j];
		}
		for(int j=0;j<topicn;j++)
		{
			f1<<setprecision(4)<<(alpha1[j] + uxk[i][j])/summer<<" ";
		}
		f1<<endl;
	}
	f1.close(); 

	ofstream f2("rtopics.txt");
	for(int i=0;i<resourcen;i++)
	{
		double summer = 0;
		for(int j=0;j<topicn;j++)
		{
			summer = summer + alpha2[j] + rxk[i][j];
		}
		for(int j=0;j<topicn;j++)
		{
			f2<<setprecision(4)<<(alpha2[j] + rxk[i][j])/summer<<" ";
		}
		f2<<endl;
	}
	f2.close();

	ofstream f3("jwords.txt");
	for(int i=0;i<topicn;i++)
	{
		double summer = 0;
		for(int j=0;j<vocabindex.size();j++)
		{
			summer = summer + beta[j] + vxk[j][i];
		}
		for(int j=0;j<vocabindex.size();j++)
		{
			f3<<setprecision(4)<<(beta[j] + vxk[j][i])/summer<<" ";
			//cout<<vxk[j][i]<<" "<<summer<<endl;
		}
		f3<<endl;
	}
	f3.close();

	ifstream f4;
	ofstream f5("jtopics.txt");
	if(iters%2 == 0)
		f4.open("topics2.txt");
	else
		f4.open("topics1.txt");
	string temp;
	while(getline(f4,temp))
	{
		istringstream instr(temp);
		
		int topic;
		int topicnumber[topicn];
		for(int i=0;i<topicn;i++)
			topicnumber[i] = 0;
	
		while(instr>>topic)
		{
			topicnumber[topic]++;
		}

		double summer = 0;
		for(int i=0;i<topicn;i++)
		{
			summer = summer + topicnumber[i] + alpha1[i];
		}

		for(int i=0;i<topicn;i++)
			f5<<setprecision(4)<<(topicnumber[i] + alpha1[i])/summer<<" ";
		f5<<endl;
	}
	f4.close();
	f5.close();

	int top=10;
	ofstream f6("top10.txt");
	for(int i=0;i<topicn;i++)
	{
		multimap<double,int> topsort;
		for(int j=0;j<vocabindex.size();j++)
		{
			topsort.insert(pair<double,int>(vxk[j][i],j));			
		}

		multimap<double,int>::reverse_iterator iter1=topsort.rbegin();
		for(int j=0;j<top;j++)
		{
			f6<<indexvocab[iter1->second]<<" ";
			iter1++;
		}
		f6<<endl;

		topsort.clear();
	}
	f6.close();

	ofstream f7("jtopic_size.txt");
	for(int k=0;k<topicn;k++){
		int topic_size = 0;
		for(int i=0;i<vocabindex.size();i++){
			topic_size += vxk[i][k];	
		}
		f7<<topic_size<<endl;
	}
	f7.close();
}

void rmtmpfiles()
{
	remove("vocabindex.txt");
	remove("filesnumeric.txt");
	remove("topics1.txt");
	//remove("topics2.txt");
}

int main(int argc, char*argv[])
{
	if(argc != 4)
	{
		cout<<"input error"<<endl;	
		cout<<"Format: ./jlda files.txt filesid.txt topicnumber"<<endl;
	}
	topicn = atoi(argv[argc-1]);
	getVocabulary(argv[argc-3]);
	indexContents(argv[argc-3]);
	getUsersResources(argv[argc-2]);

	// parameters
	double alpha1[topicn];
	double alpha2[topicn];
	double beta[vocabindex.size()];
	int **uxk = new int*[usern];
	int **rxk = new int*[resourcen];
	int **vxk = new int*[vocabindex.size()];
	double denominator[topicn];

	for(int i=0;i<usern;i++)
		uxk[i] = new int[topicn];
	for(int i=0;i<resourcen;i++)
		rxk[i] = new int[topicn];
	for(int i=0;i<vocabindex.size();i++)
		vxk[i] = new int[topicn];

	srandom(time(NULL));

	// initial topics
	initialTopics();
	initialParams(argv[argc-2],alpha1,alpha2,beta,uxk,rxk,vxk,denominator);
	cout<<"initial ok"<<endl;

	// jlda
	int iters = 500;
	cout<<"jlda"<<endl;
	jlda(argv[argc-2],alpha1,alpha2,beta,uxk,rxk,vxk,denominator,iters);

	// save results
	saveres(alpha1,alpha2,beta,uxk,rxk,vxk,iters);

	// remove temp files
	rmtmpfiles();

	// deconstruct
	for(int i=0;i<usern;i++)
		delete uxk[i];
	for(int i=0;i<resourcen;i++)
		delete rxk[i];
	for(int i=0;i<vocabindex.size();i++)
		delete vxk[i];

	delete uxk;
	delete rxk;
	delete vxk;

	return 1;
}




