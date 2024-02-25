#include <QCoreApplication>
#include <QMutex>
#include <QWaitCondition>
#include <QTime>
#include <QThread>
#include <QDebug>

// QWaitCondition
// 和QMutex结合使用可以在条件达到后立即唤醒其他线程

#define DATASIZE 100
#define TEMPSIZE 8

// 定义当前货柜
int buffer[TEMPSIZE];
int numUsable = 0; // 定义当前柜子中的数量
int index = 0;

QMutex mutex;

// 定义QWaitCondition
QWaitCondition bufferNotFull;
QWaitCondition bufferNotEmpty;


class Producer: public QThread{
protected:
    void run(){
        for (int i = 0; i < DATASIZE; i++){
        // 进来先锁线程
        mutex.lock();
        if (numUsable == TEMPSIZE){// 缓冲区没有空位
            qDebug() << "缓冲区没有空位";
            bufferNotFull.wait(&mutex);
        }
        buffer[i%TEMPSIZE] = i+1;
        ++numUsable;
        qDebug() << "生产者生产了一个产品" << currentThreadId() << i+1;
        mutex.unlock();
        // 增加随机等待时间
        msleep((qrand()%5+1)*100);
        bufferNotEmpty.wakeOne(); // 激活非空线程

        }
    }
};

class Customer: public QThread{
protected:
    void run(){

        while(1){
            mutex.lock();
            if (numUsable <= 0){
                qDebug() << "没有货物可以消费";
                bufferNotEmpty.wait(&mutex); // 等待可以消费的货物
            }
            qDebug() << "消费者消费了" << currentThreadId() <<buffer[index];
            index = (++index)%TEMPSIZE;
            numUsable--;
            mutex.unlock();
            msleep((qrand()%5+1)*100);
            bufferNotFull.wakeOne();

        }
    }
};



int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qsrand(QTime::currentTime().msec());

    // 创建一个生产者，两个消费者
    Producer producer;
    Customer cus1;
    Customer cus2;
    producer.start();
    cus1.start();
    cus2.start();

    producer.wait();
    cus1.wait();
    cus2.wait();

    return a.exec();
}
