#include <iostream>
#include <fstream>
#include <time.h>
using namespace std;

#define BUFSIZE 1024
#define FRAMESIZE  3 
#define FILESIZE 500000
int totalIO = 0;  //总IO次数
int leftFrame = BUFSIZE; //剩余空闲的frame

//一个frame
typedef struct bFrame
{
	char field[FRAMESIZE];
}bFrame;
//LRU中的元素，记录frame信息
typedef struct BCB
{
	int page_id; 
	bool dirty; 
	bFrame* frame_address; 
	BCB* former; 
	BCB* next;
} BCB;
BCB* head=NULL; //指向LRU链表头部
BCB* tail=NULL;  //指向LRU链表尾部
//openHash中的元素，记录page信息及page在LRU中的位置
typedef struct frame
{
	int page_id;
	BCB* LRU_pos;
	frame* next;
} frame;
frame* hTable[BUFSIZE]; //openHash表

//生成数据文件
void generateFile(int num)
{
	fstream output("test.dbf");
	srand(time(0));
	for (int i = 0; i < num; i++)
	{
		//随机生成content字符串
		char content[FRAMESIZE];
		for (int j = 0; j < FRAMESIZE; j++) {
			content[j] = rand() % 94 + 33;
		}
		output.write((char*)content, FRAMESIZE * sizeof(char));
	}
	output.close();
}
//buffer写出到磁盘页
void writePage(BCB* slot, int page_id)
{
	fstream f("test.dbf");
	f.seekp(page_id*FRAMESIZE, ios::beg); //定位到该页
	f.write((char*)(slot->frame_address->field), FRAMESIZE);
	totalIO++;	
	f.close();
}
//磁盘页读入buffer
void readPage(BCB* slot, int page_id)
{
	fstream f("test.dbf");
	f.seekg(page_id * FRAMESIZE, ios::beg);
	f.read((char*)(slot->frame_address->field), FRAMESIZE);
	totalIO++;
	f.close();
}
//读buffer页
void read(bFrame* frame)
{
	//for (int i = 0; i < FRAMESIZE; i++) 
	//{
	//	cout << frame->field[i];
	//}
}
//在程序结束时将buffer中的脏页写回磁盘
//bingo
int writeDirtys()
{
	int finalWrite = 0; //最终写回的页数
	for (BCB* start = head; start != NULL; )
	{
		if (start->dirty == true)
		{
			//cout << "writePage " << start->page_id << endl;
			writePage(start, start->page_id);
			finalWrite++;
		}
		BCB* temp = start->next;
		free(start);
		start = temp;
	}
	return finalWrite;
}
//Hash函数,pageid取余BUFSIZE
//bingo
int hashFunction(int page_id)
{
	return page_id % BUFSIZE;
}
//搜索hTable[hashFunction(page_id)]，看有无page_id，若有则返回其在LRU中的位置，否则返回空指针null
//bingo
frame* searchHashTable(int page_id)
{
	frame* temp = hTable[hashFunction(page_id)];
	while (temp != NULL&&temp->page_id!=page_id)
	{
		temp = temp->next;
	}
	return temp;
}
//将页插入hTable中
//bingo
void insertHashEle(int page_id, BCB* LRUpos)
{
	frame* temp = hTable[hashFunction(page_id)];
	if (temp == NULL) 
	{
		temp=hTable[hashFunction(page_id)] = (frame*)malloc(sizeof(frame));
		temp->page_id = page_id;
		temp->LRU_pos = LRUpos;
		temp->next = NULL;
	}
	else 
	{
		while (temp->next != NULL)
		{
			temp = temp->next;
		}
		temp->next = (frame*)malloc(sizeof(frame));
		temp->next->LRU_pos = LRUpos;
		temp->next->page_id = page_id;
		temp->next->next = NULL;
	}
}
//将openHash中该page移除
//bingo
void removeHashEle(int page_id)
{
	frame* temp = hTable[hashFunction(page_id)];
	if (temp->page_id == page_id)
	{
		hTable[hashFunction(page_id)] = temp->next;
		free(temp);
	}
	else 
	{
		while (temp->next->page_id != page_id)
		{
			temp = temp->next;
		}
		frame* toFree = temp->next;
		temp->next = temp->next->next;
		free(toFree);
	}
}
//将新页插入LRU队首，动态生成frame地址，更新BCB* tail位置，返回刚插入的元素在LRU链表中的地址 
//bingo
BCB* insertLRUEle(int page_id)
{
	BCB* latestFrame = (BCB*)malloc(sizeof(BCB));
	bFrame* temp = (bFrame*)malloc(sizeof(bFrame));
	latestFrame->frame_address = temp;
	latestFrame->dirty = false;
	latestFrame->page_id = page_id;
	latestFrame->former = NULL;
	if (head == NULL) //在第一个页放入buffer时
	{
		latestFrame->next = NULL;
		tail = latestFrame;
	}
	else 
	{
		latestFrame->next = head;
		head->former = latestFrame;
	}
	readPage(latestFrame, page_id);
	head = latestFrame;
	return head;
}
//更新LRU列表，将最近访问的元素移到队首
//bingo
void updateLRU(BCB* LRUpos)
{
	//若被访问的是队首元素
	if (LRUpos == head)
	{
		return;
	}
	//若被访问的为队尾元素，更新tail值
	if (LRUpos == tail)
	{
		tail = LRUpos->former;
		LRUpos->next = head;
		LRUpos->former->next = NULL;
		LRUpos->former = NULL;
		head->former = LRUpos;
		head = LRUpos;
	}
	//若被访问的是中间元素
	else
	{
		LRUpos->former->next = LRUpos->next;
		LRUpos->next->former = LRUpos->former;
		LRUpos->former = NULL;
		LRUpos->next = head;
		head->former = LRUpos;
		head = LRUpos;
	}	
}
//将LRU队尾元素的page换为当前请求的page
//bingo
BCB* selectVictim(int page_id)
{
	BCB* pos=tail;
	if (pos->dirty == true)
		writePage(tail, page_id);
	removeHashEle(pos->page_id);  //将openHash中该page移除
	insertHashEle(page_id, pos); //将新页插入openHash中
	pos->page_id = page_id;
	pos->dirty = false;
	readPage(pos, page_id);  //读入新页
	updateLRU(pos);
	return pos;
}
//置脏位为1
//bingo
void setDirty(BCB* LRU_address)
{
	LRU_address->dirty = 1;
}
//查询page_id是否在buffer中，有则返回，无则替换
//bingo
BCB* FixPage(int page_id)
{
	//查找openHash中是否有此page
	frame* position = searchHashTable(page_id);
	if (position != NULL)  //该页已在buffer中
	{
		updateLRU(position->LRU_pos);  //更新页在LRU中的位置
		return position->LRU_pos;
	}
    else  //若页不在buffer中
    {
        if (leftFrame == 0)  //若buffer满了，则选页替换
            return selectVictim(page_id);
        else   //若buffer没满
		{
			//分配新的frame地址给页，将此页插入LRU表
			BCB* newFrame = insertLRUEle(page_id);
			//在openHash中登记此页的信息
			insertHashEle(page_id, newFrame);
			leftFrame--;
			return newFrame;
		}
	}
}

/*测试用*/
void printLRU()
{
	cout << "LRUlist: ";
	BCB* iter;
	for (iter = head; iter!=tail; iter=iter->next)
	{
		cout << iter->page_id << " " << iter->frame_address << " ";
	}
	cout << iter->page_id << " " << iter->frame_address << endl;	
}
void printhTable()
{
	cout << "hTable: " << endl;
	for (int i = 0; i < BUFSIZE; i++)
	{
		cout << i << "th buffer: ";
		for (frame* iter = hTable[i]; iter != NULL; iter = iter->next)
		{
			cout << iter->page_id << " " << iter->LRU_pos << ", ";
		}
		cout << endl;
	}
	cout << endl;
}
/*测试用*/

int main()
{
	/*第一次运行时执行*/
	//生成500000条记录，放入文件data.dbf
	//generateFile(FILESIZE);
	ifstream instream;
	string fileName = "data-5w-50w-zipf.txt";
	instream.open(fileName.data());
	int page_id,operation;
	int totallines = 0;
	char c;
	while (!instream.eof())
	{
		instream >> operation;
		instream >> c;
		instream >> page_id;
		totallines++;
		BCB* LRU_address = FixPage(page_id);
		if (operation)  //若为写操作
		{
			setDirty(LRU_address);
		}
		else  //若为读操作 
		{
			read(LRU_address->frame_address);
		}
	}
	cout << "总共有" << totallines << "条记录" << endl;
	cout << "程序结束时写回" << writeDirtys() << "条记录" << endl;
	cout << "totalIO=" << totalIO << endl;
}

