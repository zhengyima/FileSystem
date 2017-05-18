#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>      
#define BLOCK_SIZE 1024   
#define SYS_SIZE 1024*1024   
#define FILE_MAX 80
        
FILE *f;
struct filsys{                      
    int s_nfree;                    
    long s_free[100];               
    int s_ninode;                   
    int s_inode[96];                
}*p;
struct index_block{                 
    int s_nfree;
    long s_free[96];
}q;
struct filelist{                    
    char name[14];                  
    int inode_num;                  
}file;

struct fcb{                         
    char name[12];                  
    int inode_num;                  
    int used;                       
    int i_size;                     
    int block_amount;               
    long i_addr[16];                
}fcb[5],*ptr;
/*struct open_file_table{             
    int offset;                     
    struct fcb* f_node;             
}table[5];
struct fd{                          
    struct open_file_table* t_node; 
}fd[5];*/


struct Boot_BLK{//引导块 
	int inode_BLK_free;
	int data_BLK_free;
}Boot_BLK,b1;

struct Super_BLK{
	int  BLK_Size;
	char Type;
	char Flag;
	int  Rinode_num;
}Super_BLK,s1;

struct inode{
    char FileName[65];
	int   BLK_Start;
	int   BLK_Amount;
	time_t Time_Create;
	int   Size;          
}inode,inodes[16];

struct dir{
	char dirname[9];
	char name[80][9];
	int  inode_num[80];
	int  BLK_Amount[80];
}d0;

struct open_file_table{             
    char filename[5][10];  
	bool isopen[5];        
}table;

struct n_file{
	char content[8*1024];
}n_file;



int pow(int rn,int en){
	int i;
	int num=rn;
	for(i=0;i<en-1;i++){
		num=num*rn;
	}
	return num;
}

void GET_BLK(void* buf,int blk_no,int size){//size参数很重要 
	if(blk_no>=SYS_SIZE/BLOCK_SIZE){
		printf("the block %d that you want to read not exists\n",blk_no);
		return ;
	}
	fseek(f,blk_no*BLOCK_SIZE,SEEK_SET);
	fread(buf,size,1,f);
}

void PUT_BLK(void* buf,int blk_no){//*
	if(blk_no>=SYS_SIZE/BLOCK_SIZE){
		printf("the block %d that you want to write not exists\n",blk_no);
		return ;
	}
	fseek(f,blk_no*BLOCK_SIZE,SEEK_SET);
	fwrite(buf,BLOCK_SIZE,1,f);
} 

FILE* apply_room(char *sys_name){//*
    f = fopen(sys_name,"w+b");       //创建一个新的可读写的二进制文件
    fseek(f,SYS_SIZE,SEEK_SET);     
    fputc(EOF, f);                  
    fclose(f);
    return fopen(sys_name,"r+b");     //打开一个可读写的二进制文件
}
void char2bit(char* charmap,char* bitmap){//低位-高位 低位-高位 
	int  length=strlen(charmap);
	int  i=0,j=0,i1=0;
	char c;
	int  ascii;   
	for(i=0;i<length;i++){
		for(i1=0;i1<8;i++){
			bitmap[8*i+i1]='0';
		}
		ascii=(int)charmap[i]; 
		while(!ascii){
			bitmap[j]=ascii%2+48;
			ascii=ascii/2;
			j++;
		}
	}
	bitmap[j]='\0';
} 
void bit2char(char* bitmap,char* charmap){
	int length=strlen(bitmap)-1;
	if(length%8){
		printf("the length of bitmap error!");
		return ;
	}
	int i=0;
	int sum=0;
	while(i<length){
		sum=sum+pow(2,i%8);
		charmap[i/8]=(char)sum;
		i++;
	}
	charmap[i/8]='\0';
}
void PUT_DIR(){
//	printf("test\n");
	char dbbitmap[81];
	char ibitmap[11];
	char chardbbitmap[641];
	char charibitmap[81];
	int i,inode_add,db_add,p=0;
	time_t t;
	
	GET_BLK(&Boot_BLK,0,sizeof(Boot_BLK));
	if(Boot_BLK.data_BLK_free<=0){
		printf("the disk is full and cannot put in more");
		return;
	}
	Boot_BLK.data_BLK_free--;
    Boot_BLK.inode_BLK_free--;
	PUT_BLK(&Boot_BLK,0);
	
	GET_BLK(dbbitmap,2,81*sizeof(char));
	bit2char(dbbitmap,chardbbitmap);
	for(i=0;i<640;i++){
		if(chardbbitmap[i]=='0') break;
	}
	chardbbitmap[i]='1';
	db_add=i;
//	printf("#%d\n",db_add);
	char2bit(chardbbitmap,dbbitmap);
	PUT_BLK(dbbitmap,2);
			

	GET_BLK(ibitmap,3,11*sizeof(char));
//	printf("#%d\n",db_add);
	//printf("#%d\n",p);
	bit2char(ibitmap,charibitmap);
	for(i=0;i<80;i++){
		if(charibitmap[i]=='0') break;
	}
	charibitmap[i]='1';
	inode_add=i;
//	printf("@%d",inode_add);
	char2bit(charibitmap,ibitmap);
	PUT_BLK(ibitmap,3);
	

	GET_BLK(inodes,4+inode_add/16,16*sizeof(inodes));
	inodes[inode_add%16].BLK_Amount=1;
	inodes[inode_add%16].BLK_Start=9+db_add;
	
	
//	printf("%d\n",d);
//	printf("%d\n",&d0);
	
	strcpy(inodes[inode_add%16].FileName,d0.dirname);
//	printf("test");
	inodes[inode_add%16].Size=sizeof(inodes[inode_add%16]);
	inodes[inode_add%16].Time_Create=time(&t);
	PUT_BLK(inodes,4+inode_add/16);
//	printf("test %d\n",inode_add);
	PUT_BLK(&d0,9+db_add);
	
}
void PUT_NFILE(){
	char dbbitmap[81];
	char ibitmap[11];
	char chardbbitmap[641];
	char charibitmap[81];
	int i,inode_add,db_add,p=0;
	unsigned int sizeoffile=strlen(n_file.content);
	int BLK_am=sizeoffile/1024;
	time_t t;
	
	GET_BLK(&Boot_BLK,0,sizeof(Boot_BLK));
	if(Boot_BLK.data_BLK_free<BLK_am){
		printf("the disk is full and cannot put in more");
		return;
	}
	Boot_BLK.data_BLK_free -= BLK_am;
    Boot_BLK.inode_BLK_free--;
	PUT_BLK(&Boot_BLK,0);
	
	GET_BLK(ibitmap,3,11*sizeof(char));
//	printf("#%d\n",db_add);
	//printf("#%d\n",p);
	bit2char(ibitmap,charibitmap);
	for(i=0;i<80;i++){
		if(charibitmap[i]=='0') break;
	}
	charibitmap[i]='1';
	inode_add=i;
//	printf("@%d",inode_add);
	char2bit(charibitmap,ibitmap);
	PUT_BLK(ibitmap,3);
	
	//该找相应数量个数据块并把文件内容写入，并写入inode 
	
	
	
	
	
}
void init(){  
    time_t t;
    Boot_BLK.data_BLK_free=FILE_MAX;
    Boot_BLK.inode_BLK_free=FILE_MAX;
    PUT_BLK(&Boot_BLK,0);
    
    Super_BLK.BLK_Size=BLOCK_SIZE;
    Super_BLK.Flag='E';
    Super_BLK.Rinode_num=1+1+1+1;
    Super_BLK.Type='?';
    PUT_BLK(&Super_BLK,1);
    
    char bitmapb[81]={'0'};
    bitmapb[80]='\0';
    PUT_BLK(bitmapb,2);
    
	char bitmapi[11]={'0'};
	bitmapi[10]='\0';
	PUT_BLK(bitmapi,3);
	
	
//	dir d0;
	strcpy(d0.dirname,"root");
/*	PUT_BLK(&d0,9);
	
	inodes[0].BLK_Amount=1;
	inodes[0].BLK_Start=9;
	strcpy(inodes[0].FileName,"root");
	inodes[0].Size=0;
	time(&t);
	inodes[0].Time_Create=t;
	PUT_BLK(&inodes,4);
	
	char charmap[641]={'0'};
	charmap[640]='\0';
	charmap[0]='1';
	char2bit(charmap,bitmapb);
	PUT_BLK(bitmapb,2);
	
	char imap[81]={'0'};
	imap[80]='\0';
	imap[0]='1';
	char2bit(imap,bitmapi);
	PUT_BLK(bitmapi,3);
*/
    PUT_DIR();	
	
 //   int j;
 //   long i;
  //  p->s_nfree=1;
  //  p->s_free[0]=0;
  //  p->s_ninode=96;
 //   for(i=0;i<96;i++)
 //      p->s_inode[i]=-1;            
  //  for(i=22;i<=SYS_SIZE/BLOCK_SIZE;i++)
  //     myfree(i);                   
   // j=p->s_nfree+1;
//    while(j<100)
  //     p->s_free[j++]=0;            
  //  fseek(f,0,SEEK_SET);
   // fwrite(&Boot_BLK,sizeof(struct filsys),1,f);
}
bool hasopen(char* filename){
	int i;
	for(i=0;i<5;i++){
		if(!strcmp(filename,table.filename[i])&&table.isopen){
			return true;
		}	
	}
	return false;
}
void openfile(char* filename){
	if(hasopen(filename)){
		printf("the file %s cannot open since has been open",filename);
		return;
	}
	int i;
	for(i=0;i<5;i++){
		if(!table.isopen[i]){
			strcpy(table.filename[i],filename);
			table.isopen[i]=true;
			printf("the file %s open success",filename);
			return ;
		}
	}
}



int split(char dst[][80], char* str, const char* spl)
{
    int n = 0;
    char *result = NULL;
    result = strtok(str, spl);
    while( result != NULL )
    {
        strcpy(dst[n++], result);
        result = strtok(NULL, spl);
    }
    return n;
}



int findBLK(char* str){//返回块号 
	char addr[5][80];
	int numoflayers;
	int inode_num=0;
	int data_num;
	int i=0,j=0,k=0;
	bool hasfound=false;
	numoflayers=split(addr,str,"/");
	
	for(i=0;i<numoflayers-1;i++){
		/*for(j=0;j<5;j++){
			GET_BLK(inodes,4+j,sizeof(inodes));
			for(k=0;k<15;k++){
				if(!strcmp(inodes[k].FileName,addr[i])){
					hasfound=true;inode_num=inodes[k].BLK_Start;break;
				}
			} 
			if(hasfound) break;
		}
		if(!hasfound) return -1;*/
		GET_BLK(inodes,4+inode_num/16,sizeof(inodes));
		data_num=inodes[inode_num%16].BLK_Start;
		GET_BLK(&d0,data_num,sizeof(d0));
		for(j=0;j<80;j++){
			if(!strcmp(addr[i+1],d0.name[j])){
				inode_num=d0.inode_num[j];
			}
		}
		if(j>=80) return -1;
	}
	GET_BLK(inodes,4+inode_num/16,sizeof(inodes));
	data_num=inodes[inode_num%16].BLK_Start;
	return data_num;	
} 




void myfree(long block_num)         
{  
 int i;
    if(p->s_nfree<100)              
 {             
       p->s_free[p->s_nfree]=block_num;
       p->s_nfree++;
    }
 else       
 {                          
       q.s_nfree=p->s_nfree;        
       for(i=0;i<100;i++)
          q.s_free[i]=p->s_free[i];
       fseek(f,(block_num-1)*BLOCK_SIZE,SEEK_SET);
       fwrite(&q,sizeof(struct index_block),1,f); 
       p->s_nfree=1;                
       p->s_free[0]=block_num;
    }
}
long myalloc()      
{                     
    int i;
    long a;
    p->s_nfree--;
    if(p->s_nfree==0){              
       a=p->s_free[0];
       fseek(f,(a-1)*BLOCK_SIZE,SEEK_SET);
       fread(&q,sizeof(struct index_block),1,f);
       p->s_nfree=q.s_nfree;        
       for(i=0;i<100;i++)
          p->s_free[i]=q.s_free[i];
       return a;
    }else return p->s_free[p->s_nfree];
}

int ialloc(){                       
    int i=0;
    while(p->s_inode[i]>=0) i++;    
    p->s_inode[i]=0;                
    p->s_ninode--;
    return i;
}
int namei(char *name)             
{  
 int k=0;
    while(k<96){
       if(p->s_inode[k]!=-1){     
          fseek(f,BLOCK_SIZE+k*16,SEEK_SET);
          fread(&file,sizeof(struct filelist),1,f);
          if(!strcmp(file.name,name))
             return file.inode_num;
       }
       k++;
    };
    return -1;                   
}
int name_i(char *name)           
{  
 int k=0;
    do
 {
       if(fcb[k].used==1)          
    {      
          if(!strcmp(fcb[k].name,name)) 
            return fcb[k].inode_num;
       }
       k++;
    }while(k<5);
    return -1;                   
}
void create()                    
{  
    /* i,inode_num;
    long long t;
    char name[12];
    printf("input file name:");
    scanf("%s",name);
    getchar();
    if(namei(name)!=-1) printf("file exited!\n");
    else
 {                           
       inode_num=ialloc();      
       strcpy(file.name,name);
       file.inode_num=inode_num;
       fseek(f,BLOCK_SIZE+inode_num*16,SEEK_SET);
       fwrite(&file,sizeof(struct filelist),1,f);
       inode.i_size=0;                     
       inode.block_amount=0;
       for(i=0;i<16;i++) inode.i_addr[i]=0;
       time(&t);
       strcpy(inode.create_time,ctime(&t));
       fseek(f,4*BLOCK_SIZE+inode_num*sizeof(struct inode),SEEK_SET);
       fwrite(&inode,sizeof(struct inode),1,f);
       p->s_inode[inode_num]=0;    
       printf("create sucessfully!\n");
    }*/
}
void display()               
{ /* 
 int k;
    for(k=0;k<96;k++)
 {
       if(p->s_inode[k]>=0)  
    {
          fseek(f,BLOCK_SIZE+k*16,SEEK_SET);
          fread(&file,sizeof(struct filelist),1,f); 
          printf("%s     ",file.name);
          fseek(f,4*BLOCK_SIZE+file.inode_num*sizeof(struct inode),SEEK_SET);
          fread(&inode,sizeof(struct inode),1,f);  
          printf("size:?     ",inode.i_size);
          printf("time:%s\n",inode.create_time);
       }
    };
    printf("\n");
    getchar();*/
}
void open_file()                
{  /*
    int i=0,j=0,k=0;
    int m,n;
    char name[12];
    printf("input file's name:");
    scanf("%s",name);
    getchar();
    n=namei(name);             
    if(n==-1) printf("file not exits!\n");
    else if(p->s_inode[n]>0) printf("file have already been opened!\n");
    else{
       while(fcb[i].used==1) i++;    
       while(table[j].f_node) j++;   
       while(fd[k].t_node) k++;      
       fd[k].t_node=&table[j];       
       table[j].f_node=&fcb[i];      
       strcpy(fcb[i].name,name);
       fcb[i].inode_num=n;
       fcb[i].used=1;
       fseek(f,4*BLOCK_SIZE+n*sizeof(struct inode),SEEK_SET);
       fread(&inode,sizeof(struct inode),1,f);
       fcb[i].i_size=inode.i_size;
       fcb[i].block_amount=inode.block_amount;
       for(m=0;m<16;m++) fcb[i].i_addr[m]=inode.i_addr[m];
       p->s_inode[n]=k+100;          
       printf("file is open!\n");
    }*/
}
void write_file()            
{  /*
 int k,block_amount,n,size=0,i=0;
    long block_num;
    char ch,name[12];
    printf("input file's name:");
    scanf("%s",name);
    getchar();
    n=name_i(name);          
    if(n==-1) printf("file not exits or not open!\n");
    else{
       k=p->s_inode[n]-100;  
       ptr=fd[k].t_node->f_node;
       while(i<ptr->block_amount)
    {
          block_num=ptr->i_addr[i];
          myfree(block_num);
          i++;
       }
       block_amount=0;
       printf("input the context of the file:(end the file with '*')\n");
       while((ch=getchar())!='*'&&block_amount<16){
          size++;
          if(size==1){          
             block_num=myalloc();   
             inode.i_addr[block_amount]=ptr->i_addr[block_amount]=block_num;
             block_amount++;
             fseek(f,(block_num-1)*BLOCK_SIZE,SEEK_SET);
          }
          fputc(ch,f);
       }
       getchar();
       inode.i_size=ptr->i_size=size;
       inode.block_amount=ptr->block_amount=block_amount;
       fseek(f,4*BLOCK_SIZE+n*sizeof(struct inode),SEEK_SET);
       fwrite(&inode,sizeof(struct inode),1,f);   
    }*/
}
void read_file()           
{  /*
 int k,n,block_amount,size;
    int i=0;
    long block_num;
    char name[12],buf[512];
    printf("input file's name:");
    scanf("%s",name);
    getchar();
    n=name_i(name);        
    if(n==-1) printf("file not exits or not open!");
    else
 {
       k=p->s_inode[n]-100;
       ptr=fd[k].t_node->f_node;
       size=ptr->i_size;
       block_amount=ptr->block_amount;
       for(i=0;i<block_amount;i++)
    {
          block_num=ptr->i_addr[i];                    
          fseek(f,(block_num-1)*BLOCK_SIZE,SEEK_SET);  
          if(size>512) {fread(buf,sizeof(char),512,f);   size=size-512;}
          else
    {
             fread(buf,sizeof(char),size,f);   
             buf[size]='\0';
          }
          printf("%s",buf);
       }
    }
    printf("\n");*/
}
void del_file()             
{  /*
 int n,i=0;
    long block_num;
    char name[12];
    printf("input file's name:");
    scanf("%s",name);
    getchar();
    n=namei(name);          
    if(n==-1) printf("file not exits!\n");     
    else if(p->s_inode[n]>0) printf("file is open now!Close it first\n");
    else{
       p->s_inode[n]=-1;    
       fseek(f,4*BLOCK_SIZE+n*sizeof(struct inode),SEEK_SET);
       fread(&inode,sizeof(struct inode),1,f);  
       while(i<inode.block_amount){
          block_num=inode.i_addr[i];
          myfree(block_num);     
          i++;
       }
       strcpy(file.name,"");     
       file.inode_num=0;
       fseek(f,BLOCK_SIZE+n*16,SEEK_SET);
       fwrite(&file,sizeof(struct filelist),1,f);
       printf("file is deleted\n");
    }*/
}
void close_file()           
{/*  
 int k,n;
    char name[12];
    printf("input file's name:");
    scanf("%s",name);
    getchar();
    n=name_i(name);         
    if(n==-1) printf("file not exits or not open\n");
    else{
       k=p->s_inode[n]-100;  
       fd[k].t_node->f_node->used=0;   
       fd[k].t_node->f_node=NULL;      
       fd[k].t_node=NULL;              
       p->s_inode[n]=0;      
       printf("file is closed!\n");
    }*/
}
void myexit()            
{/*  
 int i=0;
    char ch;
    while(fcb[i].used==0) i++;   
    if(i<5){             
       getchar();
       printf("some files are still open!!!\n");
       printf("input 'q' to quit or other key to return:\n");
       scanf("%c",&ch);
       if(ch=='q'){      
          while(i<5){
             if(fcb[i].used==1) p->s_inode[fcb[i].inode_num]=0;
             i++;
          }
          fseek(f,0,SEEK_SET);
          fwrite(p,sizeof(struct filsys),1,f);
          exit(0);
       }
       getchar();
    }else{               
       fseek(f,0,SEEK_SET);
       fwrite(p,sizeof(struct filsys),1,f);
       exit(0);
    }*/
}

int main()
{ 
    char buf[1024];
    unsigned int n1;
   /* f = fopen("test.txt","w+b");       //创建一个新的可读写的二进制文件
    fseek(f,SYS_SIZE,SEEK_SET);     
    fputc(EOF, f);        
	fseek(f,0,SEEK_SET)  ;   
	fputs("asdasdasdagasdasdasduibadjkasbdjsabdjasbdoasjbdas",f);
	GETBLK(buf,0);
	printf("%c",buf[0]);
	PUTBLK("asdasd",2);
    fclose(f);*/
    f=apply_room("g");
    init();
    GET_BLK(&buf,2,sizeof(buf));
    printf("%c",buf[0]);
    
  /*  char des[80][80];
	char src[80];
	scanf("%s",src);
	printf("%d",split(des,src,"/")); */
   // printf("%x",n1);
    return 0;
    //return fopen(sys_name,"r+b");     //打开一个可读写的二进制文件
  /* 
 int i;
    char ch,sys_name[15];
    p=(struct filsys *)malloc(sizeof(struct filsys));  
    while(1)
 {
       printf("1:Create a new file system\n");
       printf("2:open an existed file system\n");
       printf("choose:");
       if((ch=getchar())=='1')        
    {       
          printf("input file system's name:");
          scanf("%s",sys_name);
          getchar();
          f=apply_room(sys_name);     
          init();                     
          break;                       //这里的break用来跳出while(1)的循环
       }
    else if(ch=='2')      
    {            
          printf("input file system's name:");
          scanf("%s",sys_name);
          getchar();
          f=fopen(sys_name,"r+b");
          fseek(f,0,SEEK_SET);
          fread(p,sizeof(struct filsys),1,f);    
          break;
       }
       else
    {
          printf("wrong input!\n");
          getchar();
       }
    };
    for(i=0;i<5;i++)                      
 {            
       fcb[i].used=0;
       table[i].f_node=NULL;
       fd[i].t_node=NULL;
    }
    while(1)
 {
       printf("--------------------------------------------------------------------\n");
       printf("1:create 2:open 3:write 4:read 5:close 6:delete 7:display 8:exit\n");
       printf("choose:");
       switch(getchar()-'0')    
    {     
    case 1:create();break;
    case 2:open_file();break;
    case 3:write_file();break;
    case 4:read_file();break;
    case 5:close_file();break;
    case 6:del_file();break;
    case 7:display();break;
    case 8:myexit();break;
    default:getchar(); printf("wrong input!\n"); break;
       }
    };
    free(p);
    fclose(f);
    */
}
