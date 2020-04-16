#include "mq.h"

/**
 * @brief   Creation of the message queue id.
 * @return  Message queue id
 */
int mqid(){
    int msqid;
    key_t key;

    /*  Create text file that will be used to create the key */
    if(system("touch msgq.txt")<0){
        perror("Error en crear archivo.");
        exit(EXIT_FAILURE);
    };
    
    /*  Create key */
    if ((key = ftok("msgq.txt", 'B')) == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    /*  Creation of the message queue id, with their respective permissions */
    if ((msqid = msgget(key, PERMS | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    return msqid;
}

/**
 * @brief   Wrapper to receive messages from the message queue
 * @param   msqid   Message queue ide
 * @param   msg     Message received from message queue
 * @param   mtype   Message type that identifies the process from which the message should be received
 * @return  String with the message extracted from the message queue
 */
char * rcv_msg(int msqid, char * msg, long mtype){
    /*  Pointer and allocate for structure  */  
    struct my_msgbuf *buf;
    buf = malloc(sizeof(struct my_msgbuf));
    
    /*  Receive message */
    if (msgrcv(msqid, buf, sizeof(buf->mtext), mtype, 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }
    
    /*  Copy message to string */
    sprintf(msg, "%s", buf->mtext);
    return msg;
}

/**
 * @brief   Wrapper to send messages to the message queue
 * @param   msqid   Message queue ide
 * @param   msg     Message received from message queue
 * @param   mtype   Message type that identifies the process from which the message should be received
 */
void snd_msg(int msqid, char * msg, long mtype){
    /*  String size to be sent */
    size_t len = strlen(msg);
    /*  Pointer and allocate for structure  */
    struct my_msgbuf *buf;
    buf = malloc(sizeof(struct my_msgbuf));

    /*  Message type    */
    buf->mtype = mtype;
    
    /*  Copy the string to the field of the structure   */
    sprintf(buf->mtext, "%s", msg);

    /*  Send message    */
    if (msgsnd(msqid, buf, len+1, 0) == -1) /* +1 for '\0' */
        perror("msgsnd");
}

/**
 * @brief   Get info from message queue
 * @param   msqid 
 */
void mq_info(int msqid){
    struct msqid_ds buf;
    msgctl(msqid, IPC_STAT, &buf);
    unsigned int msg = (unsigned int)(buf.msg_qnum);
    printf("Mensajes en la cola: %u\n", msg);
    printf("pid last msgsnd = %ld\n", (long) buf.msg_lspid);
    printf("pid last msgrcv = %ld\n", (long) buf.msg_lrpid);
}