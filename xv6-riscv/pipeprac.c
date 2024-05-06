#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/sem.h>

#define KEY 1234
#define SEM_NUM 0
#define BUFSIZE 1

int main() {
    int pipefd1[2]; //子から親へのパイプ
    int pipefd2[2]; //親から子へのパイプ
    char buf1[BUFSIZE]; //pipefd1で使用するデータの格納変数
    char buf2[BUFSIZE]; //pipefd2で使用するデータの格納変数
    int semid;
    struct sembuf sem_lock = {SEM_NUM, -1, 0}; // セマフォを減らしてロック
    struct sembuf sem_unlock = {SEM_NUM, 1, 0}; // セマフォを増やしてアンロック
    int pid;
    int count = 0;
    time_t start, end;
    char p;
    char c;
    int sec;

    // パイプを作成する
    if (pipe(pipefd1) < 0) {
        fprintf(stderr, "pipe failed\n");
        return 1;
    }
    if (pipe(pipefd2) < 0) {
        fprintf(stderr, "pipe failed\n");
        return 1;
    }

    // セマフォを作成
    semid = semget(KEY, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    // セマフォの初期値を1に設定（ロックされていない状態）
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("semctl SETVAL");
        exit(EXIT_FAILURE);
    }

    // 時間計測を開始する
    time(&start);
    time(&end);
    sec = end - start;
    printf("sec = %d\n", sec);
    // プロセスをフォークする
    if ((pid = fork()) < 0) {
        fprintf(stderr, "fork failed\n");
        return 1;
    }
    strncpy(buf1, "A", BUFSIZE);
    while (1 > sec) {
        if (pid == 0) {  // 子プロセス
            // セマフォをロック
            if (semop(semid, &sem_lock, 1) == -1) {
                perror("semop lock");
                exit(EXIT_FAILURE);
            }

            close(pipefd1[0]);  // 子から親へのパイプの、読み取り側を閉じる
            write(pipefd1[1], buf1, BUFSIZE);  // データを書き込む
            close(pipefd1[1]);  // 子から親へのパイプの、書き込み側を閉じる

            // セマフォをアンロック
            if (semop(semid, &sem_unlock, 1) == -1) {
                perror("semop unlock");
                exit(EXIT_FAILURE);
            }
            close(pipefd2[1]);  // 親から子へのパイプの、書き込み側を閉じる
            read(pipefd2[0], buf2, BUFSIZE); // データを読み取る
            close(pipefd2[0]);  // 親から子へのパイプの、読み取り側を閉じる

            strncpy(buf1, buf2, BUFSIZE);
            printf("Data at Child Process is %s\n", buf1);

            count++;  // カウントを増やす
            // 1秒ごとにカウントを表示
            printf("Count: %d\n", count);

        } else {  // 親プロセス
            close(pipefd1[1]);  // 子から親へのパイプの、書き込み側を閉じる
            read(pipefd1[0], buf1, BUFSIZE);  // データを読み取る
            close(pipefd1[0]);  // 子から親へのパイプの、読み取り側を閉じる

            strncpy(buf2, buf1, BUFSIZE);
            printf("Data at Parents Process is %s\n", buf2);

            // セマフォをロック
            if (semop(semid, &sem_lock, 1) == -1) {
                perror("semop lock");
                exit(EXIT_FAILURE);
            }

            close(pipefd2[0]);  // 親から子へのパイプの、読み取り側を閉じる
            write(pipefd2[1], buf2, BUFSIZE);  // データを書き込む
            close(pipefd2[1]);  // 親から子へのパイプの、書き込み側を閉じる

            // セマフォをアンロック
            if (semop(semid, &sem_unlock, 1) == -1) {
                perror("semop unlock");
                exit(EXIT_FAILURE);
            }
        }
        time(&end);
        sec = end - start;
    }

    return 0;
}
