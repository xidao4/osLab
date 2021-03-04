#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<vector>
#include<iostream>
using namespace std;
typedef unsigned char u8;	//1字节
typedef unsigned short u16;	//2字节
typedef unsigned int u32;	//4字节
struct Entry{
    u8 name[8];
    u8 suffix[3];
    u8 attr;
    u8 useless[10];
    u16 wrtTime;
    u16 wrtDate;
    u16 fstClus;
    u32 fileSize;
};
class Node{
public:
    string name;
    int type; //0: dir    1:.txt
    int size;
    vector<Node*> sons;
    int startClus;
    string path;//eg. /HOUSE/ROOM/  /NJU/CS/
};
struct BPB{
    u8 bootjmp[3];
    u8 oem_name[8];
    u16 bytes_per_sector;
    u8 sectors_per_cluster;
    u16 reserved_sector_count;
    u8 table_count;
    u16 root_entry_count;
    u16 total_sectors_16;
    u8 media_type;
    u16 table_size_16;
    u16 sectors_per_track;
    u16 head_side_count;
    u32 hidden_sector_count;
    u32 total_sectors_32;
};
u16 BytsPerSec;	//每扇区字节数
int SecPerClus;	//每簇扇区数
u16 RsvdSecCnt;	//Boot记录占用的扇区数
int TblCnt;	//FAT表个数
u16 RootEntCnt;	//根目录最大文件数
u16 TtlSec16;//number of total sectors in the logical volume
u16 TblSz16;//number of sectors per FAT
FILE* fat12;
u8 file[80*18*512*2];
Node* root;

extern "C"{
    void myPrint(const char*,const int,const int);
}
void printInt(int a){
    char res[10];
    int b=a,len=1;
    while(b/10>0){
        len++;
        b/=10;
    }
    for(int i=len-1;i>=0;i--){
        res[i]='0'+a%10;
        a/=10;
    }
    res[len]='\0';
    myPrint(res,len,0);
}
void println(const char *addr,int len,int red){
    myPrint(addr,len,red);
    char c='\n';
    myPrint(&c,1,0);
}
void printOnlyln(){
    char c='\n';
    myPrint(&c,1,0);
}
void printWithoutln(const char* addr,int len,int red){
    myPrint(addr,len,red);
}
int get_int(int offset,int len){
    int result = 0, i;
    for (i = offset + len - 1; i >= offset; --i) {
        result = result * 256 + file[i];
    }
    return result;
}
int getFATValue(int num) {
	int base=RsvdSecCnt*BytsPerSec;
    int table_value=get_int(base+num+num/2,2);
    if (num & 0x0001) {
        return table_value >> 4;
    } else {
        return table_value & 0x0FFF;
    }
}
int filterEntry(struct Entry* entryPtr){
    if(entryPtr->name[0]=='\0') return -1; //empty entry! continue
        //过滤非目标文件
		int j;
		int boolean = 0;
		for (j = 0; j < 8; j++) {
			if (!(((entryPtr->name[j] >= 48) && (entryPtr->name[j] <= 57)) ||
				((entryPtr->name[j] >= 65) && (entryPtr->name[j] <= 90)) ||
				((entryPtr->name[j] >= 97) && (entryPtr->name[j] <= 122)) ||
				(entryPtr->name[j] == ' '))) {
				boolean = 1;	//非英文及数字、空格
				break;
			}
		}
		if (boolean == 1) return -1;	//非目标文件不输出
        return 1;
}
void getStructure(int dirBaseByt,struct Entry* entryPtr,Node* fa){
    int is_root=0,check,clus=fa->startClus;
    if(clus==0) is_root=1;
    for(int i=dirBaseByt;;i+=32){
        if(is_root==1 && i>=dirBaseByt+32*RootEntCnt) break;
        else if(is_root==0 && i>=dirBaseByt+512){
            if(clus=getFATValue(clus)>=0xff7) break;
            else {
                i=(1+9*2+14)*512+(clus-2)*512;//attention
                dirBaseByt=i;
            }
        }
        check=fseek(fat12,i,SEEK_SET);
        //if(check==-1)cout<<"fseek failed"<<endl;
        check=fread(entryPtr,1,32,fat12);
        //if(check!=32)cout<<"fread failed"<<endl;

        if(filterEntry(entryPtr)==-1) continue;

        Node * son=new Node;
        if((entryPtr->attr & 0x10)==0){//.txt
            son->type=1;

            son->name="";
            for(int i=0;i<8;i++){
                if(entryPtr->name[i]!=' ') son->name+=entryPtr->name[i];
            }
            son->name+=".TXT";

            son->startClus=entryPtr->fstClus;
            son->size=entryPtr->fileSize;

            fa->sons.push_back(son);
        }else{//dir
            son->type=0;

            son->name="";
            for(int i=0;i<8;i++){
                if(entryPtr->name[i]!=' ') son->name+=entryPtr->name[i];
            }
            
            son->path=fa->path+son->name+"/";

            son->startClus=entryPtr->fstClus;

            fa->sons.push_back(son);

            int sonClus=son->startClus;
            int sonDirBaseByt=(1+9*2+14)*512+(sonClus-2)*512;
            struct Entry newEntry;
            struct Entry* newEntryPtr=&newEntry;
            getStructure(sonDirBaseByt,newEntryPtr,son);
        }     
    }
}
Node* findDir(string dirName,Node* curr){
    if (curr->name==dirName)
        return curr;
    else{
        for(int i=0;i<curr->sons.size();i++){
            Node* ret= findDir(dirName,curr->sons[i]);
            if(ret!=NULL) return ret;
        }
        return NULL;
    }
}
void printLS(Node* tar){
    string msg=tar->path+":";
    println(msg.c_str(),msg.size(),0);

    if(tar->name!="") {
        msg=". .. ";
        printWithoutln(msg.c_str(),msg.size(),1);
    }

    vector<Node*> sons=tar->sons;
    for(int i=0;i<sons.size();i++){
        msg=sons[i]->name+" ";     
        if(sons[i]->type==0)
            printWithoutln(msg.c_str(),msg.size(),1);
        else
            printWithoutln(msg.c_str(),msg.size(),0);            
    }
    printOnlyln();

    for(int i=0;i<sons.size();i++){
        if(sons[i]->type==0) printLS(sons[i]);
    }
    
}
int getDirNum(Node* tar){
    int ret=0;
    for(int i=0;i<tar->sons.size();i++){
        if (tar->sons[i]->type==0) ret++;
    }
    return ret;
}
int getTXTNum(Node* tar){
    int ret=0;
    for(int i=0;i<tar->sons.size();i++){
        if (tar->sons[i]->type==1) ret++;
    }
    return ret;
}
void printLSL(Node* tar){
    string msg=tar->path+" ";
    printWithoutln(msg.c_str(),msg.size(),0);
    printInt(getDirNum(tar));
    msg=" ";
    printWithoutln(msg.c_str(),msg.size(),0);
    printInt(getTXTNum(tar));
    msg=":";
    println(msg.c_str(),msg.size(),0);
    //cout<<tar->path<<" "<<getDirNum(tar)<<" "<<getTXTNum(tar)<<":"<<endl;

    if(tar->name!="") {
        msg=".";
        println(msg.c_str(),msg.size(),1);
        msg="..";
        println(msg.c_str(),msg.size(),1);
        //cout<<"."<<endl<<".."<<endl;
    }

    vector<Node*> sons=tar->sons;
    for(int i=0;i<sons.size();i++){
        if(sons[i]->type==0){//dir
            msg=sons[i]->name+" ";
            printWithoutln(msg.c_str(),msg.size(),1);
            printInt(getDirNum(sons[i]));
            msg=" ";
            printWithoutln(msg.c_str(),msg.size(),0);
            printInt(getTXTNum(sons[i]));
            printOnlyln();
            //cout<<sons[i]->name<<" "<<getDirNum(sons[i])<<" "<<getTXTNum(sons[i])<<endl; 
        }else{//file
            msg=sons[i]->name+" ";
            printWithoutln(msg.c_str(),msg.size(),0);
            printInt(sons[i]->size);
            printOnlyln();
            //cout<< sons[i]->name<<" "<<sons[i]->size<<endl; 
        }       
    }
    printOnlyln();

    for(int i=0;i<sons.size();i++){
        if(sons[i]->type==0) printLSL(sons[i]);
    }
}
string getLastDir(string dirName){
    string ret="";
    int i;
    for(i=dirName.length()-1;i>=0;i--){
        if(dirName[i]=='/') break;
        ret=dirName[i]+ret;
    }
    if (ret==""){
        for(int j=i-1;j>=0;j--){
            if(dirName[j]=='/') break;
            ret=dirName[j]+ret;
        }
    }
    return ret;
}
void cmdLS(string dirName){
    dirName=getLastDir(dirName);
    Node * tar=findDir(dirName,root);
    if(tar==NULL) {
        string err="path of ls is wrongly given!!";
        println(err.c_str(),err.size(),0);
    }
    printLS(tar);
}
void cmdLSL(string dirName){
    dirName=getLastDir(dirName);
    Node* tar=findDir(dirName,root);
    if(tar==NULL){
        string err="path of ls -l is wrongly given!!";
        println(err.c_str(),err.size(),0);
    }
    printLSL(tar);
}
int getFileStartClus(string name,Node* node){
    if(node->name==name) return node->startClus;
    vector<Node*> sons=node->sons;
    for(int i=0;i<sons.size();i++){
        int ret=getFileStartClus(name,sons[i]);
        if(ret!=-1) return ret;
    }
    return -1;
}
void getFileAndPrint(int fstClus){
    int curr_cluster=fstClus;
    int check;
    while(true){
        int root_dir_sectors=(224*32+512-1)/512;
        int first_data_sector=1+2*9+root_dir_sectors;
        int first_sector_of_cluster=((curr_cluster-2)*1)+first_data_sector;
        int baseByte=first_sector_of_cluster*512;

        check=fseek(fat12,baseByte,SEEK_SET);
        //if(check==-1) cout<<"fseek failed"<<endl;
        char content[513]={0};
        check=fread(content,1,512,fat12);
        //if(check!=512) cout<<"fread failed"<<endl;
        printWithoutln(content,512,0);
        //cout<<content;

        int value=getFATValue(curr_cluster);           
        if(value>=0xFF) {
            printOnlyln();
            //cout<<endl;
            break; 
        }
        if(value==0xFF7) break;           //bad cluster~~

        curr_cluster=value;
    } 
}
void cmdCAT(string wholeName){
    string fileName="";
    for(int i=wholeName.size()-1;i>=0;i--){
        if(wholeName[i]=='/') break;
        fileName=wholeName[i]+fileName;
    }
    int fstClus=getFileStartClus(fileName,root);
    if(fstClus==-1) {
        string err="filename doesn't exist!";
        println(err.c_str(),err.size(),0);
    }
    else getFileAndPrint(fstClus);
}
bool has_suffix(const string &str,const string &suffix){
    return str.size()>=suffix.size()&& str.compare(str.size()-suffix.size(),suffix.size(),suffix)==0;
}
bool hasIllegal(string para){
    for(int i=1;i<para.length();i++){
        if(para[i]=='l') continue;
        else return true;
    }
    return false;
}
int main(){
    fat12=fopen("/home/xidao/a2.img","rb");
    fread(file,1,2880*512,fat12);

    BytsPerSec=get_int(11,2);       //512
    SecPerClus = file[13];          //1
	RsvdSecCnt = get_int(14,2);     //1
	TblCnt = file[16];              //2
	RootEntCnt = get_int(17,2);     //224
    TtlSec16=get_int(19,2);         //2880
	TblSz16 = get_int(22,2);        //9

    root=new Node();
    root->name="";
    root->type=0;
    root->startClus=0;
    root->path="/";
    
    struct Entry entry;
    struct Entry* entryPtr=&entry;
    int rootDirBaseByt=(1+9*2)*512;//attention
    getStructure(rootDirBaseByt,entryPtr,root);

    string instr;

    while(true){
        println("please enter the command:",25,0);
        getline(cin,instr);

        vector<string> cmdParameter;
        string curr="";
        for(int i=0;i<instr.length();i++){
            if(instr.at(i)==' '){
                cmdParameter.push_back(curr);
                curr="";
            }else{
                curr+=instr.at(i);
            }
        }
        cmdParameter.push_back(curr);

        if(cmdParameter[0]=="cat"){
            if(cmdParameter.size()!=2){
                string error="the number of cat command's parameter is wrong!!";
                println(error.c_str(),error.length(),0);
            }
            else {
                if(has_suffix(cmdParameter[1],".TXT")) cmdCAT(cmdParameter[1]);
                else {
                    string error="cat command's parameter must be a file!!";
                    println(error.c_str(),error.length(),0);
                }
                
            }
        }else if(cmdParameter[0]=="ls"){
            if(cmdParameter.size()==1) cmdLS("");
            else {
                bool isValid=true;

                string dirName="";
                int Lflag=0;

                for(int i=1;i<cmdParameter.size();i++){
                    if(cmdParameter[i][0]!='-'){
                        if(dirName!=""){
                            string err="path appears twice!!";
                            println(err.c_str(),err.length(),0);
                            isValid=false;
                            break;
                        }else{
                            dirName=cmdParameter[i];
                        }
                    }else{
                        if(hasIllegal(cmdParameter[i])){
                            isValid=false;
                            string err="-l cmd is wrongly used!!";
                            println(err.c_str(),err.length(),0);
                            break;
                        }else{
                            Lflag=1;
                        } 
                    }
                }
                if(isValid){
                    if(Lflag==1) cmdLSL(dirName);
                    else cmdLS(dirName);
                }                
            }  
        }else if (cmdParameter[0]=="exit"){
            break;
        }
        else {
            string err="wrong command!!";
            println(err.c_str(),err.length(),0);
        }
    }
}