#include "common.h"

static int shm_id , sem_id;
void* addr;

void p_handle(int shm_id_)
{
    assert((semop(shm_id_,&p,1))!=-1);
}

void v_handle(int shm_id_)
{
    assert((semop(shm_id_,&v,1))!=-1);
}

void send(const Message *message)
{
    p_handle(sem_id);
    assert(memcpy(addr,message, message->size) == addr);
    assert(shmdt(addr) == -1);
    v_handle(sem_id);
}

const Message *recv()
{
    static Message *m = (Message *)malloc(MESSAGE_SIZES[4]);
    assert(memcpy(m,addr, sizeof(Message)) == addr);
    assert(shmdt(addr) == -1);
    return m;
}

int main()
{
    union semun setval;
    setval.val = 1;
    Message *m2 = (Message *)malloc(MESSAGE_SIZES[4]);
    shm_id = shmget(0, MESSAGE_SIZES[4], 0666);
    assert(shm_id != -1);
    addr = shmat(shm_id,NULL,0);
    assert(addr != NULL);
    sem_id = semget(0, 1, IPC_CREAT);
    assert(sem_id != -1);
    assert(semctl(sem_id, 0, SETVAL, setval)!=-1);
    while (true)
    {
        p_handle(sem_id);
        const Message *m1 = recv();
        v_handle(sem_id);
        assert(m1->checksum == crc32(m1));
        memcpy(m2, m1, m1->size); // 拷贝m1至m2
        m2->payload[0]++;         // 第一个字符加一
        m2->checksum = crc32(m2); // 更新校验和
        send(m2);
    }

    return 0;
}