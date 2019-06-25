#include <bits/stdc++.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <ios>
#include <stdlib.h>
#include <stdio.h>

#define PB push_back
using namespace std;

/*Defining the structure TreeNode to record the temporal occurrence information of an item in a transactional database. */
class TreeNode{
	public:
		int item;
		list<TreeNode*> children;	
		TreeNode* parent;
		vector< pair <int, int> > clusterItem;
		int supportItem;
		TreeNode() {}
		TreeNode(TreeNode* ptr){
			parent = ptr;
			supportItem = 0;
			item=0;
			children.clear();
		}

		~TreeNode() {
			children.clear();
		}
};

/* PfListEntry class is used to maintain PF-list. */
class PfListEntry : public TreeNode{
	public:			
		int freq,per,idl;
		bool valid;		
		PfListEntry() { idl=0,freq=0,per=0; valid=false; }
		PfListEntry(int f, int p) {
			freq=f,per=p;
		} 
		~PfListEntry() {} 
} ;


/* Structure of PF-tree.  */
class Tree{
	public:
		TreeNode* root;
		map<int,PfListEntry> PfList;
		map<int,int> supportNode;
		map<int,list<TreeNode *> > itemNodeList;
		vector < pair <int,int> > OneFreqItemsPair;
		map<int, vector <pair <int, int> > > clusterNode; 
		map<int,int> OneFreqItemsMap;
		Tree(){
			root= new TreeNode(NULL);		
			PfList.clear();
			OneFreqItemsPair.clear();		
			OneFreqItemsMap.clear();		
			itemNodeList.clear();
			//itemTransList.clear();
		}

		~Tree() { 
			delete root;	
			PfList.clear();
			OneFreqItemsPair.clear();		
			OneFreqItemsMap.clear();		
			itemNodeList.clear();
			//itemTransList.clear();
		}
};

/* Global Variables */
double minSup=0.0,maxPer=0.0; //express as value between 0 and 1
ifstream inputFile1,inputFile2;  // pointer to input dataset file
int numTrans=0;  // numOfTransactions
int numTrans1 = 0;
vector<int> OneFreqItems;
int debug=0,numPatterns=0;
ofstream outputFile;
vector<int> resultant;
double countMemory=0,numNodesPfTree=0;
map<string, int>Hash;
vector<string> ReverseHash;
int randUse,randUse1;
vector <pair<int, int> > merged;

/* Function Defintions */ 
int upper_ceil(int numTrans);
void outputPfList(Tree*);
void outputFinalPfList(Tree*);
void populatePfListHashing(Tree*,string);
void updateMinSupportMaxPeriod();
void sortPfList(Tree*);
void pruneAndSortPfList(Tree*);
int sortFuncDec(pair<int,int>,pair<int,int>);
void outputOneFreqItems(Tree*);
Tree* createPfTree(Tree*,string);
void printTree(TreeNode*);
void minePfPatterns(Tree*);
int sortFuncInc(pair<int,int>,pair<int,int>);
void helperFunc(Tree* , vector<int>);
int satisfyConditions(int,int);
void printPfPatterns(vector<int>,int, int);
Tree* createConditionalTree(Tree*, int);
void computePfList(Tree*);
Tree* pruneTailNodes(Tree*,int);
Tree* insertBranch(Tree*,TreeNode*);
int isLinear(TreeNode*);
void genLinear(TreeNode*,vector<int>);
void calculateMemory(Tree*);
void recMemory(TreeNode*);
void process_mem_usage(double &);

/* 
    main function which accepts the following input parameters inputfile, outputfile, minSup and maxPer.
*/
int main(int argc, char **argv){			
	string fileName;
	fileName = argv[1];
	//cin >> minSup >> maxPer;  // in percentage
	minSup = atof(argv[2]),maxPer=atof(argv[3]);
	numTrans1=atoi(argv[4]);
	char outFile[100];
	sprintf(outFile,"../Outputs/patterns_PF_new_%s_%f_%f_uday.txt",fileName.c_str(),minSup,maxPer);
	//printf("%f,%f,",minSup,maxPer);
	//minSup = 1,maxPer=40; // hardcoding
	if(debug) cout << fileName << endl ; 

	clock_t initial,middle,final;
	initial=clock();

	Tree *tree = new Tree();

	populatePfListHashing(tree,fileName);
	if(debug) outputPfList(tree);
	updateMinSupportMaxPeriod();
	pruneAndSortPfList(tree);
	if(debug)
		outputOneFreqItems(tree);
	middle=clock();
	float pfListTime = ( (float)(middle)-(float)(initial) )/CLOCKS_PER_SEC;


	tree = createPfTree(tree,fileName);
	if(debug) printTree(tree->root);

	OneFreqItems.clear();


	outputFile.open(outFile);

	minePfPatterns(tree);
	final=clock();
	float mineTime=( (float)(final)-(float)(middle) )/CLOCKS_PER_SEC;


	double rss;
	process_mem_usage(rss);
	rss = rss/1024;

	calculateMemory(tree);

	if(debug) cout << "Number of nodes in Pf-Tree: " << numNodesPfTree << endl;
	countMemory = countMemory / (1024*1024);
	char ch='%';
	printf("%.2f%c,%.2f%c,",100*(minSup/numTrans),ch,100*(maxPer/numTrans),ch);
	printf("%d,%f,%f,%f,%f\n",numPatterns,pfListTime,pfListTime+mineTime,countMemory,rss);
	return 0;
}

/*

Updating pfList structure

*/
void populatePfListHashing(Tree *tree,string fileName)
{
	ReverseHash.clear(); Hash.clear();
	ifstream inputFile;
	string line,item;	
	inputFile1.open(fileName.c_str());	
	int index=0;
	while(getline(inputFile1, line))
	{
		numTrans++;
		istringstream split(line);
		for(item; getline(split,item,' ');)
		{	
			if(Hash.find(item) == Hash.end() ){	// first time		
				tree->PfList[index].freq=1;
				tree->PfList[index].per=numTrans;
				tree->PfList[index].idl=numTrans;

				ReverseHash.push_back(item);
				Hash[item]=index++;
			}	
			else{
				tree->PfList[Hash[item]].freq++;								
				if( (numTrans - tree->PfList[Hash[item]].idl) > tree->PfList[Hash[item]].per)
					tree->PfList[Hash[item]].per = numTrans - tree->PfList[Hash[item]].idl ;
				tree->PfList[Hash[item]].idl=numTrans;
			}
			//tree->itemTransList[Hash[item]].push_back(numTrans);
			/*if(tree->supportNode.find(Hash[item])==tree->supportNode.end())
			  {
			  tree->supportNode[Hash[item]]=1;
			  }
			  else
			  {
			  tree->supportNode[Hash[item]]+=1;
			  }*/
		}		
	}

	if(debug) for(map<string,int>::iterator it=Hash.begin();it!=Hash.end();it++) 	cout << (*it).first << " " << (*it).second << endl;

	// Final Updation of period values	
	for(map <int,PfListEntry>::iterator it=tree->PfList.begin();it!=tree->PfList.end();it++)
	{		
		if ((*it).second.per < numTrans-(*it).second.idl)
			(*it).second.per = numTrans-(*it).second.idl;
	}
}	


void outputPfList(Tree *tree)
{
	cout << "In outputPfList\n";	
	for(map <int,PfListEntry>::iterator it=tree->PfList.begin();it!=tree->PfList.end();it++)
		cout << (*it).first << " " << (*it).second.freq << " " << (*it).second.per << endl;	
}

void updateMinSupportMaxPeriod(){
	minSup = (minSup*numTrans1*1.0)/100;
	maxPer = (maxPer*numTrans1*1.0)/100;
	maxPer = floor(maxPer);
	if(debug) cout << minSup << " " << maxPer << endl ;
}

void pruneAndSortPfList(Tree *tree){	
	for(map <int,PfListEntry>::iterator it=tree->PfList.begin();it!=tree->PfList.end();it++)
	{
		if( (*it).second.freq >= minSup && (*it).second.per <= maxPer){
			tree->OneFreqItemsPair.PB(make_pair((*it).first,(*it).second.freq));
			(*it).second.valid = true;
		}
	}
	//cout << tree->OneFreqItems.size() << endl;
	sort(tree->OneFreqItemsPair.begin(),tree->OneFreqItemsPair.end(),sortFuncDec);
	int n = int (tree->OneFreqItemsPair.size());
	for(int i=0;i<n;i++)
		tree->OneFreqItemsMap[tree->OneFreqItemsPair[i].first] = tree->OneFreqItemsPair[i].second;
}

void outputOneFreqItems(Tree *tree){	
	int n = int (tree->OneFreqItemsPair.size());
	cout<<"In output one frequent"<<endl;
	for(int i=0;i<n;i++)
	{
		cout << tree->OneFreqItemsPair[i].first << " " << tree->OneFreqItemsPair[i].second << endl;	
	}
}

void outputFinalPfList(Tree *tree)
{
	cout << "In outputPfList\n";	
	for(map <int,PfListEntry>::iterator it=tree->PfList.begin();it!=tree->PfList.end();it++)
		if((*it).second.valid)
			cout << (*it).first << " " << (*it).second.freq << " " << (*it).second.per << endl;	
}

int sortFuncDec(pair<int,int> a, pair<int,int> b){	
	if(b.second == a.second)
		return a.first < b.first;
	return a.second > b.second;
}


/*
Scanning the database second time and constructing PFTree.
*/

Tree* createPfTree(Tree *tree,string fileName){
	string line,item;	
	vector < pair<int,int> > PfList;	PfList.clear();	
	list <TreeNode *> tempV;	
	int transID=0;
	TreeNode* node;
	inputFile2.open(fileName.c_str());		
	while(getline(inputFile2, line))
	{
		transID++;		
		istringstream split(line);
		for(item; getline(split,item,' ');){			
			if( (tree->OneFreqItemsMap).find(Hash[item]) != (tree->OneFreqItemsMap).end() )
				PfList.PB(make_pair(Hash[item],tree->OneFreqItemsMap[Hash[item]]));
		}
		sort(PfList.begin(),PfList.end(),sortFuncDec);

		int index=0,match=0;		
		node = tree->root;
		while(index < int(PfList.size()) ) {
			match=0;
			for( list<TreeNode *>::iterator it_node=(node->children).begin();it_node!=(node->children).end();it_node++){
				if( (*it_node)->item == PfList[index].first )	{
					node = (*it_node);
					match=1;
					index++;
					break;
				}					
			}
			if(match==0)
				break;
		}

		while(index < int(PfList.size()) ) {
			TreeNode *tempNode = new TreeNode(node);
			tempNode->item = PfList[index].first;
			(node->children).PB(tempNode);
			node = tempNode;

			if( (tree->itemNodeList).find(PfList[index].first) == (tree->itemNodeList).end() ){
				tempV.clear();
				tempV.PB(node);
				(tree->itemNodeList)[PfList[index].first] = tempV;
			}
			else{
				(tree->itemNodeList)[PfList[index].first].PB(node);
			}
			index++;
		}

		if( int(PfList.size()) > 0)
		{
			randUse = node->clusterItem.size();
			if(node->clusterItem.size()==0)
			{
				node->supportItem=1;
				node->clusterItem.push_back(make_pair(transID,transID));
			}
			else
			{
				node->supportItem++;
				if((node->clusterItem[randUse-1].second+maxPer)>=transID)
				{
					node->clusterItem[randUse-1].second = transID;
				}
				else
				{
					node->clusterItem.push_back(make_pair(transID,transID));
				}
			}
		}
		PfList.clear();
	}
	tree->OneFreqItemsMap.clear();
	return tree;
}

void printTree(TreeNode *node)
{
	cout << node->item << " ";
	if( int(node->clusterItem.size()) >0 )
		cout<<"tlist: ";
	for (vector<pair<int,int> >::iterator vi=node->clusterItem.begin();vi!=(node->clusterItem).end();vi++)
	{
		cout<<vi->first<<" "<<vi->second<<" ";
		cout<<endl;
	}
	cout<<node->supportItem;
	cout<<endl;
	for (list <TreeNode *>::iterator it_node=(node->children).begin();it_node!=(node->children).end();it_node++)
		printTree(*it_node);	
	return;
}

void minePfPatterns(Tree *tree){
	sort(tree->OneFreqItemsPair.begin(),tree->OneFreqItemsPair.end(),sortFuncInc);

	for(vector< pair<int,int> >:: iterator it = tree->OneFreqItemsPair.begin();it!=tree->OneFreqItemsPair.end();it++)
		OneFreqItems.PB( (*it).first);

	vector<int> tempV;
	while( int(OneFreqItems.size())>0 ){
		tempV.clear();
		tempV.PB(*(OneFreqItems.begin()));		
		helperFunc(tree,tempV);		
		OneFreqItems.erase(OneFreqItems.begin());
		//break;
	}	
}

int sortFuncInc(pair<int,int> a, pair<int,int> b){	
	if(b.second == a.second)
		return a.first > b.first;
	return a.second < b.second;
}

void helperFunc(Tree *tree, vector<int> itemVec){
	if(debug) cout << "In helperFunc for item " << itemVec[0] << endl;

	if(debug) cout << "itemVector:\n";
	if(debug) for (int i = 0; i < int(itemVec.size()); i++)
		cout << itemVec[i] << " ";
	if(debug) cout << endl;
	if( satisfyConditions( (tree->PfList)[itemVec[0]].freq, (tree->PfList)[itemVec[0]].per ) )
		printPfPatterns(itemVec,(tree->PfList)[itemVec[0]].freq, (tree->PfList)[itemVec[0]].per);
	else{
		pruneTailNodes(tree,itemVec[0]);
		return;
	}

	Tree *conditionalTree = createConditionalTree(tree,itemVec[0]); 

	if ( ((conditionalTree->root)->children).empty() )
		return;

	if(debug) cout << "Printing Full conditional Tree for item " << itemVec[0] << endl;

	if(debug) printTree(conditionalTree->root);	

	/*	
		if ( isLinear(conditionalTree->root) )
		{
		genLinear(conditionalTree->root,itemVec);
		return;
		}*/


	vector<int> tempV;
	if(debug) cout << "Executing the last for loop in helperFunc\n";

	for (vector<int>::iterator it=OneFreqItems.begin()+1;it!=OneFreqItems.end();it++)
	{
		if(debug) 	cout << "Next item is " << *it << endl;
		if ((conditionalTree->itemNodeList)[*it].empty() ==0){		
			// Calling helpFunc for further extensions of "ij"
			// using tempV for pushing from front side in the itemVec
			tempV.clear();
			tempV=itemVec;
			tempV.insert(tempV.begin(),*it);
			helperFunc(conditionalTree,tempV);
		}
	}	
}

int satisfyConditions(int f, int p){
	return (f>=minSup && p<=maxPer);
}

void printPfPatterns(vector<int> item,int sup, int per){
	if(debug) cout << "In printPfPatterns \n";
	for(std::vector<int>::iterator it=item.begin();it!=item.end();it++)		
		outputFile << ReverseHash[*it] << " ";
	outputFile << ", " << sup << " , " << per << "\n";
	numPatterns++;

}

Tree* createConditionalTree(Tree *tree,int item){
	if(debug) cout << "In createConditionalTree\n";
	Tree* conditionalTree = new Tree();
	list<TreeNode*> itemNodes = tree->itemNodeList[item];
	TreeNode* tempNode;
	merged.clear();
	for(list<TreeNode*>::iterator it_node=itemNodes.begin();it_node!=itemNodes.end();it_node++){
		tempNode = (*it_node)->parent;
		while(tempNode->parent!=NULL){
			if(conditionalTree->clusterNode[tempNode->item].size()==0)
			{
				(conditionalTree->supportNode)[tempNode->item] = (*it_node)->supportItem;
				conditionalTree->clusterNode[tempNode->item] = (*it_node)->clusterItem;
				tempNode = tempNode->parent;
				continue;
			}
			randUse = (*it_node)->clusterItem.size();
			randUse1 = (conditionalTree->clusterNode)[tempNode->item].size();
			int p1 = 0, p2 = 0, p3 = 0;
			int v1,v2,u1,u2;
			while(p1<randUse1 && p2<randUse)
			{
				u1 =(conditionalTree->clusterNode)[tempNode->item][p1].first;
				u2 =(conditionalTree->clusterNode)[tempNode->item][p1].second;
				v1 =(*it_node)->clusterItem[p2].first;
				v2 =(*it_node)->clusterItem[p2].second;
				//cout<<u1<< " "<<u2<<" "<<v1<<" "<<v2<<" anirudh"<<endl;
				p3 = merged.size()-1;
				if(p3>=0)
				{
					if(merged[p3].second>u2)
					{
						p1++;
						continue;
					}
					if(merged[p3].second>v2)
					{
						p2++;
						continue;
					}
					if(merged[p3].second + maxPer >= u1)
					{
						merged[p3].second = u2;
						p1++;
						continue;
					}
					if(merged[p3].second + maxPer >= v1)
					{
						merged[p3].second = v2;
						p2++;
						continue;
					}
				}
				if(u1 <= v1)
				{
					if(u2 > v2)
					{
						merged.push_back(make_pair(u1,u2));
						p1++;
						p2++;
						continue;
					}
					if((u2 < v2) && (u2 > v1))
					{
						merged.push_back(make_pair(u1,v2));
						p1++;
						p2++;
						continue;
					}
					if( u2 + maxPer >= v1)
					{
						merged.push_back(make_pair(u1,v2));
						p1++;
						p2++;
						continue;
					}
					else
					{
						merged.push_back(make_pair(u1,u2));
						p1++;
						continue;
					}
				}
				if(u1 > v1)
				{
					if((u2 > v2) && (u1 < v2))
					{
						merged.push_back(make_pair(v1,u2));
						p1++;
						p2++;
						continue;
					}
					if((u2 < v2))
					{
						merged.push_back(make_pair(v1,v2));
						p1++;
						p2++;
						continue;
					}
					if( v2 + maxPer >= u1)
					{
						merged.push_back(make_pair(v1,u2));
						p1++;
						p2++;
						continue;
					}
					else
					{
						merged.push_back(make_pair(v1,v2));
						p2++;
						continue;
					}
				}
			}
			while(p1<randUse1)
			{
				u1 =(conditionalTree->clusterNode)[tempNode->item][p1].first;
				u2 =(conditionalTree->clusterNode)[tempNode->item][p1].second;
				p3 = merged.size()-1;
				if(p3>=0)
				{
					if(merged[p3].second>u2)
					{
						p1++;
						continue;
					}
					if(merged[p3].second + maxPer >= u1)
					{
						merged[p3].second = u2;
						p1++;
						continue;
					}
				}
				merged.push_back(make_pair(u1,u2));
				p1++;
			}
			while(p2<randUse)
			{
				v1 =(*it_node)->clusterItem[p2].first;
				v2 =(*it_node)->clusterItem[p2].second;
				p3 = merged.size()-1;
				if(p3>=0)
				{
					if(merged[p3].second>v2)
					{
						p2++;
						continue;
					}
					if(merged[p3].second + maxPer >= v1)
					{
						merged[p3].second = v2;
						p2++;
						continue;
					}
				}
				merged.push_back(make_pair(v1,v2));
				p2++;
			}
			(conditionalTree->clusterNode)[tempNode->item].clear();
			(conditionalTree->clusterNode)[tempNode->item]=merged;
			merged.clear();
			(conditionalTree->supportNode)[tempNode->item] += (*it_node)->supportItem;
			tempNode = tempNode->parent;
		}
	}

	computePfList(conditionalTree);

	for(list<TreeNode *>::iterator it_node=itemNodes.begin();it_node!=itemNodes.end();it_node++)
		conditionalTree=insertBranch(conditionalTree,*it_node);
	pruneTailNodes(tree,item);	
	if (debug) cout << "Before Pruning\n";
	if (debug) printTree(conditionalTree->root);
	if (debug) cout << "After Pruning\n";
	if (debug) printTree(conditionalTree->root);
	return conditionalTree;
}

void computePfList(Tree *tree){
	int per=0;
	int item;
	for(map<int,vector<pair <int,int> > >::iterator it = (tree->clusterNode).begin();it!=(tree->clusterNode).end();it++){
		randUse = it->second.size();
		if(randUse == 1)
		{
			if((it->second[0].first <= maxPer) && ((numTrans1 - it->second[0].second) <= maxPer))
			{
				per = maxPer - 1;
			}
			else
			{
				per = maxPer + 1;
			}
		}
		else
		{
			per = maxPer+1;
		}

		item = (*it).first;
		(tree->PfList)[item].freq=tree->supportNode[item];
		(tree->PfList)[item].per=per;
	}


	for(map<int,PfListEntry>::iterator it = (tree->PfList).begin();it != (tree->PfList).end();it++){
		if(satisfyConditions((*it).second.freq,(*it).second.per))
			(*it).second.valid=true;
		else
			(*it).second.valid=false;
	}

	if(debug) outputPfList(tree);
}

Tree* insertBranch(Tree* conditionalTree,TreeNode* tail)
{
	if(debug) cout << "in insertBranch \n";
	TreeNode *node=tail->parent;
	stack <TreeNode *> path;		

	// Pushing all the ancestors of item into the stack
	//	if(node->parent==NULL)
	//		return conditionalTree;
	while (node->parent!=NULL)
	{
		if(debug) 	cout << node->item << " ";
		if ( (conditionalTree->PfList)[node->item].valid )
			path.push(node);
		node=node->parent;
	}
	if(debug) 	cout << endl;

	// Converting the above path into tree
	int match=0;
	TreeNode *topNode,*tempNode;
	node=conditionalTree->root;	
	int flag=0;
	while (!path.empty())
	{		
		match=1;
		topNode=path.top();
		if(debug) cout << topNode->item << " ";
		for (list<TreeNode *>::iterator it_node=(node->children).begin();it_node!=(node->children).end();it_node++)
		{			
			if ( (*it_node)->item==topNode->item )
			{
				if(!flag){			
					flag=1;
					if(debug)  cout << "\nPartial/Full match found\n";
				}
				match=0;
				path.pop();
				node=*it_node;
				break;
			}
		}
		if(match)
			break;
	}
	if(debug) cout << "2nd part:\n";
	flag=0;
	while (!path.empty())
	{		
		if(!flag){			
			flag=1;
			if(debug) cout << "Some Part of the path is inserted for the first time\n";
		}
		topNode=path.top();
		path.pop();

		tempNode = new TreeNode(node);		
		tempNode->item=topNode->item;		
		(node->children).PB(tempNode);
		(conditionalTree->itemNodeList)[tempNode->item].PB(tempNode);		
		node=tempNode;		
	}

	merged.clear();
	int p1 = 0, p2 = 0, p3 = 0;
	int u1,u2,v1,v2;
	randUse = tail->clusterItem.size();
	randUse1 = node->clusterItem.size();
	if(node->clusterItem.size() == 0)
	{
		node->supportItem+=tail->supportItem;
		node->clusterItem = tail->clusterItem;
		if(debug)	{
			cout << "Temporary Array contents: ";
			for(vector<pair<int,int> > ::iterator v= node->clusterItem.begin();v!=node->clusterItem.end();v++)
				cout << v->first <<" "<<v->second << " ";
			cout << endl;
			cout << "In printTree for one branch\n";
			printTree(conditionalTree->root); 
		}
		return conditionalTree;
	}
	while(p1<randUse1 && p2<randUse)
	{
		u1 =node->clusterItem[p1].first;
		u2 =node->clusterItem[p1].second;
		v1 =tail->clusterItem[p2].first;
		v2 =tail->clusterItem[p2].second;
		p3 = merged.size()-1;
		if(p3>=0)
		{
			if(merged[p3].second>u2)
			{
				p1++;
				continue;
			}
			if(merged[p3].second>v2)
			{
				p2++;
				continue;
			}
			if(merged[p3].second + maxPer >= u1)
			{
				merged[p3].second = u2;
				p1++;
				continue;
			}
			if(merged[p3].second + maxPer >= v1)
			{
				merged[p3].second = v2;
				p2++;
				continue;
			}
		}
		if(u1 <= v1)
		{
			if(u2 > v2)
			{
				merged.push_back(make_pair(u1,u2));
				p1++;
				p2++;
				continue;
			}
			if((u2 < v2) && (u2 > v1))
			{
				merged.push_back(make_pair(u1,v2));
				p1++;
				p2++;
				continue;
			}
			if( u2 + maxPer >= v1)
			{
				merged.push_back(make_pair(u1,v2));
				p1++;
				p2++;
				continue;
			}
			else
			{
				merged.push_back(make_pair(u1,u2));
				p1++;
				continue;
			}
		}
		if(u1 > v1)
		{
			if((u2 > v2) && (u1 < v2))
			{
				merged.push_back(make_pair(v1,u2));
				p1++;
				p2++;
				continue;
			}
			if((u2 < v2))
			{
				merged.push_back(make_pair(v1,v2));
				p1++;
				p2++;
				continue;
			}
			if( v2 + maxPer >= u1)
			{
				merged.push_back(make_pair(v1,u2));
				p1++;
				p2++;
				continue;
			}
			else
			{
				merged.push_back(make_pair(v1,v2));
				p2++;
				continue;
			}
		}
	}
	while(p1<randUse1)
	{
		u1 =node->clusterItem[p1].first;
		u2 =node->clusterItem[p1].second;
		p3 = merged.size()-1;
		if(p3>=0)
		{
			if(merged[p3].second>u2)
			{
				p1++;
				continue;
			}
			if(merged[p3].second + maxPer >= u1)
			{
				merged[p3].second = u2;
				p1++;
				continue;
			}
		}
		merged.push_back(make_pair(u1,u2));
		p1++;
	}
	while(p2<randUse)
	{
		v1 =tail->clusterItem[p2].first;
		v2 =tail->clusterItem[p2].second;
		p3 = merged.size()-1;
		if(p3>=0)
		{
			if(merged[p3].second>v2)
			{
				p2++;
				continue;
			}
			if(merged[p3].second + maxPer >= v1)
			{
				merged[p3].second = v2;
				p2++;
				continue;
			}
		}
		merged.push_back(make_pair(v1,v2));
		p2++;
	}
	(node->clusterItem).clear();
	(node->clusterItem) = merged;
	node->supportItem+=tail->supportItem;

	if(debug)	{
		cout << "Temporary Array contents: ";
		for(vector<pair<int,int> > ::iterator v= node->clusterItem.begin();v!=node->clusterItem.end();v++)
			cout << v->first <<" "<<v->second << " ";
		cout << endl;
		cout << "In printTree for one branch\n";
		printTree(conditionalTree->root); 
	}
	return conditionalTree;
}


Tree* pruneTailNodes(Tree* conditionalTree,int item){	
	if(debug) cout << "In pruneTailNodes\n";
	TreeNode *parent;
	for (list<TreeNode *>::iterator it_node=(conditionalTree->itemNodeList)[item].begin();it_node!=(conditionalTree->itemNodeList)[item].end();it_node++)
	{
		parent=(*it_node)->parent;
		if(debug) cout << parent->item;
		merged.clear();
		int p1 = 0, p2 = 0, p3 = 0;
		int v1,v2,u1,u2;
		randUse = (*it_node)->clusterItem.size();
		randUse1 = parent->clusterItem.size();
		if(parent->clusterItem.size()==0)
		{
			parent->supportItem = (*it_node)->supportItem;
			parent->clusterItem = (*it_node)->clusterItem;
			continue;
		}
		while(p1<randUse1 && p2<randUse)
		{
			u1 =parent->clusterItem[p1].first;
			u2 =parent->clusterItem[p1].second;
			v1 =(*it_node)->clusterItem[p2].first;
			v2 =(*it_node)->clusterItem[p2].second;
			p3 = merged.size()-1;
			if(p3>=0)
			{
				if(merged[p3].second>u2)
				{
					p1++;
					continue;
				}
				if(merged[p3].second>v2)
				{
					p2++;
					continue;
				}
				if(merged[p3].second + maxPer >= u1)
				{
					merged[p3].second = u2;
					p1++;
					continue;
				}
				if(merged[p3].second + maxPer >= v1)
				{
					merged[p3].second = v2;
					p2++;
					continue;
				}
			}
			if(u1 <= v1)
			{
				if(u2 > v2)
				{
					merged.push_back(make_pair(u1,u2));
					p1++;
					p2++;
					continue;
				}
				if((u2 < v2) && (u2 > v1))
				{
					merged.push_back(make_pair(u1,v2));
					p1++;
					p2++;
					continue;
				}
				if( u2 + maxPer >= v1)
				{
					merged.push_back(make_pair(u1,v2));
					p1++;
					p2++;
					continue;
				}
				else
				{
					merged.push_back(make_pair(u1,u2));
					p1++;
					continue;
				}
			}
			if(u1 > v1)
			{
				if((u2 > v2) && (u1 < v2))
				{
					merged.push_back(make_pair(v1,u2));
					p1++;
					p2++;
					continue;
				}
				if((u2 < v2))
				{
					merged.push_back(make_pair(v1,v2));
					p1++;
					p2++;
					continue;
				}
				if( v2 + maxPer >= u1)
				{
					merged.push_back(make_pair(v1,u2));
					p1++;
					p2++;
					continue;
				}
				else
				{
					merged.push_back(make_pair(v1,v2));
					p2++;
					continue;
				}
			}
		}
		while(p1<randUse1)
		{
			u1 =parent->clusterItem[p1].first;
			u2 =parent->clusterItem[p1].second;
			p3 = merged.size()-1;
			if(p3>=0)
			{
				if(merged[p3].second>u2)
				{
					p1++;
					continue;
				}
				if(merged[p3].second + maxPer >= u1)
				{
					merged[p3].second = u2;
					p1++;
					continue;
				}
			}
			merged.push_back(make_pair(u1,u2));
			p1++;
		}
		while(p2<randUse)
		{
			v1 =(*it_node)->clusterItem[p2].first;
			v2 =(*it_node)->clusterItem[p2].second;
			p3 = merged.size()-1;
			if(p3>=0)
			{
				if(merged[p3].second>v2)
				{
					p2++;
					continue;
				}
				if(merged[p3].second + maxPer >= v1)
				{
					merged[p3].second = v2;
					p2++;
					continue;
				}
			}
			merged.push_back(make_pair(v1,v2));
			p2++;
		}
		parent->clusterItem.clear();
		parent->clusterItem = merged;
		parent->supportItem+=(*it_node)->supportItem;
	}
	(conditionalTree->itemNodeList)[item].clear();	
	return conditionalTree;
}

void calculateMemory(Tree * tree) // of only pf-list and pf-tree
{	
	countMemory=0;
	int debugMemory=0;
	if(debug){
		cout<<sizeof(TreeNode *)<<endl;
		cout<<sizeof(TreeNode )<<endl;		
		cout<<sizeof(Tree *)<<endl;
		cout<<sizeof(Tree )<<endl;
		cout<<sizeof(PfListEntry)<<endl;
	}

	// getting Memory of PFlist
	int x=0;
	for(map <int,PfListEntry>::iterator it=(tree->PfList).begin(); it!=(tree->PfList).end();it++)
	{
		if( (*it).second.valid == true)
		{
			x++;
			countMemory += sizeof( (*it).first ) + 4*sizeof(int);//sizeof(PfListEntry);
		}
	}	
	if(debugMemory)	
		cout << "Number of Items in PF-List: " << x << endl;


	// getting memory of Pf-Tree
	countMemory = sizeof(TreeNode *); // root
	// getting recursively size of tree
	numNodesPfTree=0;
	recMemory(tree->root);
	if(debugMemory)
		cout << "Number of Nodes in PF-Tree: " << numNodesPfTree << endl;
	return;
}


void recMemory(TreeNode * node)
{
	if (node==NULL)
		return;	
	numNodesPfTree++;
	countMemory += sizeof(node->item) + sizeof(TreeNode *) + sizeof(int)*(node->clusterItem).size();

	for (list <TreeNode *>::iterator it=(node->children).begin();it!=(node->children).end();it++)
		recMemory(*it);
	return ;
}

void process_mem_usage(double& resident_set)
{
	using std::ios_base;
	using std::ifstream;
	using std::string;


	resident_set = 0.0;

	// 'file' stat seems to give the most reliable results
	//
	ifstream stat_stream("/proc/self/stat",ios_base::in);

	// dummy vars for leading entries in stat that we don't care about
	//
	string pid, comm, state, ppid, pgrp, session, tty_nr;
	string tpgid, flags, minflt, cminflt, majflt, cmajflt;
	string utime, stime, cutime, cstime, priority, nice;
	string O, itrealvalue, starttime;

	// the two fields we want
	//
	unsigned long vsize;
	long rss;

	stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
		>> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
		>> utime >> stime >> cutime >> cstime >> priority >> nice
		>> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

	stat_stream.close();

	long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
	resident_set = rss * page_size_kb;
}
