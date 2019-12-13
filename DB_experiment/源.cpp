#include <iostream>
#include <fstream>
#include <time.h>
using namespace std;

#define BUFSIZE 1024
#define FRAMESIZE  3 
#define FILESIZE 500000
int totalIO = 0;  //��IO����
int leftFrame = BUFSIZE; //ʣ����е�frame

//һ��frame
typedef struct bFrame
{
	char field[FRAMESIZE];
}bFrame;
//LRU�е�Ԫ�أ���¼frame��Ϣ
typedef struct BCB
{
	int page_id; 
	bool dirty; 
	bFrame* frame_address; 
	BCB* former; 
	BCB* next;
} BCB;
BCB* head=NULL; //ָ��LRU����ͷ��
BCB* tail=NULL;  //ָ��LRU����β��
//openHash�е�Ԫ�أ���¼page��Ϣ��page��LRU�е�λ��
typedef struct frame
{
	int page_id;
	BCB* LRU_pos;
	frame* next;
} frame;
frame* hTable[BUFSIZE]; //openHash��

//���������ļ�
void generateFile(int num)
{
	fstream output("test.dbf");
	srand(time(0));
	for (int i = 0; i < num; i++)
	{
		//�������content�ַ���
		char content[FRAMESIZE];
		for (int j = 0; j < FRAMESIZE; j++) {
			content[j] = rand() % 94 + 33;
		}
		output.write((char*)content, FRAMESIZE * sizeof(char));
	}
	output.close();
}
//bufferд��������ҳ
void writePage(BCB* slot, int page_id)
{
	fstream f("test.dbf");
	f.seekp(page_id*FRAMESIZE, ios::beg); //��λ����ҳ
	f.write((char*)(slot->frame_address->field), FRAMESIZE);
	totalIO++;	
	f.close();
}
//����ҳ����buffer
void readPage(BCB* slot, int page_id)
{
	fstream f("test.dbf");
	f.seekg(page_id * FRAMESIZE, ios::beg);
	f.read((char*)(slot->frame_address->field), FRAMESIZE);
	totalIO++;
	f.close();
}
//��bufferҳ
void read(bFrame* frame)
{
	//for (int i = 0; i < FRAMESIZE; i++) 
	//{
	//	cout << frame->field[i];
	//}
}
//�ڳ������ʱ��buffer�е���ҳд�ش���
//bingo
int writeDirtys()
{
	int finalWrite = 0; //����д�ص�ҳ��
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
//Hash����,pageidȡ��BUFSIZE
//bingo
int hashFunction(int page_id)
{
	return page_id % BUFSIZE;
}
//����hTable[hashFunction(page_id)]��������page_id�������򷵻�����LRU�е�λ�ã����򷵻ؿ�ָ��null
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
//��ҳ����hTable��
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
//��openHash�и�page�Ƴ�
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
//����ҳ����LRU���ף���̬����frame��ַ������BCB* tailλ�ã����ظղ����Ԫ����LRU�����еĵ�ַ 
//bingo
BCB* insertLRUEle(int page_id)
{
	BCB* latestFrame = (BCB*)malloc(sizeof(BCB));
	bFrame* temp = (bFrame*)malloc(sizeof(bFrame));
	latestFrame->frame_address = temp;
	latestFrame->dirty = false;
	latestFrame->page_id = page_id;
	latestFrame->former = NULL;
	if (head == NULL) //�ڵ�һ��ҳ����bufferʱ
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
//����LRU�б���������ʵ�Ԫ���Ƶ�����
//bingo
void updateLRU(BCB* LRUpos)
{
	//�������ʵ��Ƕ���Ԫ��
	if (LRUpos == head)
	{
		return;
	}
	//�������ʵ�Ϊ��βԪ�أ�����tailֵ
	if (LRUpos == tail)
	{
		tail = LRUpos->former;
		LRUpos->next = head;
		LRUpos->former->next = NULL;
		LRUpos->former = NULL;
		head->former = LRUpos;
		head = LRUpos;
	}
	//�������ʵ����м�Ԫ��
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
//��LRU��βԪ�ص�page��Ϊ��ǰ�����page
//bingo
BCB* selectVictim(int page_id)
{
	BCB* pos=tail;
	if (pos->dirty == true)
		writePage(tail, page_id);
	removeHashEle(pos->page_id);  //��openHash�и�page�Ƴ�
	insertHashEle(page_id, pos); //����ҳ����openHash��
	pos->page_id = page_id;
	pos->dirty = false;
	readPage(pos, page_id);  //������ҳ
	updateLRU(pos);
	return pos;
}
//����λΪ1
//bingo
void setDirty(BCB* LRU_address)
{
	LRU_address->dirty = 1;
}
//��ѯpage_id�Ƿ���buffer�У����򷵻أ������滻
//bingo
BCB* FixPage(int page_id)
{
	//����openHash���Ƿ��д�page
	frame* position = searchHashTable(page_id);
	if (position != NULL)  //��ҳ����buffer��
	{
		updateLRU(position->LRU_pos);  //����ҳ��LRU�е�λ��
		return position->LRU_pos;
	}
    else  //��ҳ����buffer��
    {
        if (leftFrame == 0)  //��buffer���ˣ���ѡҳ�滻
            return selectVictim(page_id);
        else   //��bufferû��
		{
			//�����µ�frame��ַ��ҳ������ҳ����LRU��
			BCB* newFrame = insertLRUEle(page_id);
			//��openHash�еǼǴ�ҳ����Ϣ
			insertHashEle(page_id, newFrame);
			leftFrame--;
			return newFrame;
		}
	}
}

/*������*/
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
/*������*/

int main()
{
	/*��һ������ʱִ��*/
	//����500000����¼�������ļ�data.dbf
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
		if (operation)  //��Ϊд����
		{
			setDirty(LRU_address);
		}
		else  //��Ϊ������ 
		{
			read(LRU_address->frame_address);
		}
	}
	cout << "�ܹ���" << totallines << "����¼" << endl;
	cout << "�������ʱд��" << writeDirtys() << "����¼" << endl;
	cout << "totalIO=" << totalIO << endl;
}

