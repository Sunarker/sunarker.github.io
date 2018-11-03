#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <dirent.h>
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <iomanip>
#include <cmath>

using namespace std;

int docn = 0;
int vocabn = 0;
int topicn = 0;
map<string,int> vocabindex;
map<int,string> indexvocab;

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
	
	vocabn = vocabindex.size();

	cout<<"vocabulary size is "<<vocabindex.size()<<endl;
}

void getDocn(char*filepath)
{
	ifstream f(filepath);
	int count = 0;
	string temp;
	while(getline(f,temp))
	{
		count ++;
	}
	cout<<"the item number is "<<count<<endl;
	docn = count;
}

void indexWords(char * filepath)
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
			int index = vocabindex[word];
			f2<<index<<" ";
		}
		f2<<endl;
	}
	f1.close();
	f2.close();
}

void initialParams(double alpha[],double beta[],int **mxk,int **vxk,double denominator[])
{
	//alpha,beta
	for(int i=0;i<topicn;i++)
	{
		alpha[i] = 0.1;
	}
	for(int j=0;j<vocabn;j++)
	{
		beta[j] = 0.01;
	}

	//mxk,vxk
	for(int i=0;i<topicn;i++)
	{
		for(int j=0;j<docn;j++)
			mxk[j][i] = 0;
		for(int j=0;j<vocabn;j++)
			vxk[j][i] = 0;
	}

	ifstream f1("filesnumeric.txt");
	ofstream f2("topics1.txt");
	string temp;
	int m = 0;
	//srandom(time(0));
	while(getline(f1,temp))
	{		
		istringstream instr(temp);
		int wordind;
		while(instr>>wordind)
		{	
			int topic = (int)(((double)random()/RAND_MAX)*topicn);
			mxk[m][topic]++;
			vxk[wordind][topic]++;
			f2<<topic<<" ";
		}
		f2<<endl;
		m++;
	}
	f1.close();
	f2.close();

	//denominator
	for(int i=0;i<topicn;i++)
	{
		denominator[i] = 0;
		for(int j=0;j<vocabn;j++)
		{
			denominator[i] = denominator[i] + beta[j] + vxk[j][i];
		}
		//cout<<"denominator ["<<i<<"]:"<<denominator[i]<<endl;
	}

	cout<<"initial parameters successfully"<<endl;
}

void testError(double part1,double part2,double part3)
{
	if(part1<0)
		cout<<"part1 error:"<<part1<<endl;
	if(part2<0)
		cout<<"part2 error:"<<part2<<endl;
	if(part3<=0)
		cout<<"part3 error:"<<part3<<endl;
}

void testP(double p[])
{
	for(int i=0;i<topicn;i++)
		cout<<p[i]<<" ";
	cout<<endl;
}

void lda(int iters,double alpha[],double beta[],int **mxk,int **vxk,double denominator[])
{	
	ofstream lf("llikelihood.txt");
	for(int iter=0;iter<iters;iter++)
	{
		cout<<"iters: "<<iter<<endl;
		
		double likelihood = 0;

		ifstream f1("filesnumeric.txt");
		ifstream f2;
		ofstream f3;

		if(iter%2==0)
		{
			f2.open("topics1.txt");	
			f3.open("topics2.txt");
		}
		else
		{
			f2.open("topics2.txt");	
			f3.open("topics1.txt");
		}

		int m=0;
		string temp1,temp2;
		while(getline(f1,temp1)&&getline(f2,temp2))
		{
			istringstream instr1(temp1);
			istringstream instr2(temp2);

			int wordind,topic;
			while(instr1>>wordind && instr2>>topic)
			{								
				mxk[m][topic]--;
				vxk[wordind][topic]--;
				denominator[topic] = denominator[topic]-1;			

				double p[topicn];
				double sump = 0;
				for(int k=0;k<topicn;k++)
				{	
					double part1 = mxk[m][k] + alpha[k];
					double part2 = vxk[wordind][k] + beta[wordind];
					double part3 = denominator[k];	
					testError(part1,part2,part3);
		
					p[k]= part1 * part2/part3;
					sump += p[k];
				}
				
				//cout<<"sump:"<<sump<<endl;

				double cdf[topicn];
				for(int k=0;k<topicn;k++)
				{
					if(k==0)
						cdf[k] = p[k];
					else
						cdf[k] = cdf[k-1] + p[k];
				}

				//testP(p);
				
				double randp = ((double)random()/RAND_MAX)*cdf[topicn-1];
				//cout<<"random p: "<<randp<<endl;
				int newtopic;
				for(int k=0;k<topicn;k++)
				{
					if(randp<=cdf[k])
					{
						newtopic = k;
						likelihood = likelihood + log(p[k]/sump);
						break;
					}
				}
				
				//cout<<newtopic<<endl;
				mxk[m][newtopic]++;
				vxk[wordind][newtopic]++;
				denominator[newtopic] = denominator[newtopic] + 1;
				f3<<newtopic<<" ";
			}
			f3<<endl;
			m++;
		}
		f1.close();
		f2.close();
		f3.close();
		lf<<likelihood<<endl;
	}
	lf.close();
}

void saveres(double alpha[],double beta[],int **mxk,int **vxk)
{
	ofstream f1("topics.txt");	
	for(int i=0;i<docn;i++)
	{
		double sum=0;		
		for(int j=0;j<topicn;j++)
		{
			sum = sum + mxk[i][j] + alpha[j];
		}
		for(int j=0;j<topicn;j++)
		{
			f1<<setprecision(5)<<(mxk[i][j]+alpha[j])/sum<<" ";
		}
		f1<<endl;
	}
	f1.close();

	ofstream f2("words.txt");
	for(int i=0;i<topicn;i++)
	{
		double sum=0;		
		for(int j=0;j<vocabn;j++)
		{
			sum = sum + vxk[j][i] + beta[j];
		}
		for(int j=0;j<vocabn;j++)
		{
			f2<<setprecision(3)<<(vxk[j][i] + beta[j])/sum<<" ";
		}
		f2<<endl;
	}
	f2.close();

	int top=10;
	ofstream f3("top10.txt");
	for(int i=0;i<topicn;i++)
	{
		multimap<double,int> topsort;
		for(int j=0;j<vocabn;j++)
		{
			topsort.insert(pair<double,int>(vxk[j][i],j));			
		}

		multimap<double,int>::reverse_iterator iter1=topsort.rbegin();
		for(int j=0;j<top;j++)
		{
			f3<<indexvocab[iter1->second]<<" ";
			iter1++;
		}
		f3<<endl;

		topsort.clear();
	}
	f3.close();

	ofstream f4("ltopic_size.txt");
	for(int k=0;k<topicn;k++){
		int topic_size = 0;
		for(int i=0;i<vocabn;i++){
			topic_size += vxk[i][k];	
		}
		f4<<topic_size<<endl;
	}
	f4.close();
}

void rmtempfiles()
{
	//remove("vocabindex.txt");
	remove("filesnumeric.txt");
	remove("topics1.txt");
	remove("topics2.txt");
}

int main(int argc,char*argv[])
{
	if(argc != 3)
	{
		cout<<" input parameters error"<<endl;
		cout<<" Format: ./lda inputfile topicn"<<endl;
		return 0;
	}
	//topic number
	topicn = atoi(argv[argc-1]);
	//vocabulary and size
	getVocabulary(argv[argc-2]);
	//document number
	getDocn(argv[argc-2]);
	//index all words
	indexWords(argv[argc-2]);

	//parameters
	double alpha[topicn];
	double beta[vocabn];
	double denominator[topicn];
	int **mxk =  new int*[docn];
	int **vxk = new int*[vocabn];
	for(int i=0;i<docn;i++)
		mxk[i] = new int[topicn];
	for(int i=0;i<vocabn;i++)
		vxk[i] = new int[topicn];

	srandom(time(NULL));
//  initial parameters
	initialParams(alpha,beta,mxk,vxk,denominator);

//  lda	
	int iters=500;	
	lda(iters,alpha,beta,mxk,vxk,denominator);
	
//  save result
	saveres(alpha,beta,mxk,vxk);
	
//  remove temp files
	rmtempfiles();

// deconstruct
	for(int i=0;i<docn;i++)
		delete mxk[i];
	for(int i=0;i<vocabn;i++)
		delete vxk[i];
	delete mxk;
	delete vxk;
}
