#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define BUFSIZE 1

int main() {
    int pipefd1[2]; //子から親へのパイプ
    int pipefd2[2]; //親から子へのパイプ
    char buf1[BUFSIZE]; //pipefd1で使用するデータの格納変数
    char buf2[BUFSIZE]; //pipefd2で使用するデータの格納変数
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

            close(pipefd1[0]);  // 子から親へのパイプの、読み取り側を閉じる
            write(pipefd1[1], buf1, BUFSIZE);  // データを書き込む
            close(pipefd1[1]);  // 子から親へのパイプの、書き込み側を閉じる

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

            close(pipefd2[0]);  // 親から子へのパイプの、読み取り側を閉じる
            write(pipefd2[1], buf2, BUFSIZE);  // データを書き込む
            close(pipefd2[1]);  // 親から子へのパイプの、書き込み側を閉じる

            printf("Data at Parents Process is %s\n", buf2);

        }
        time(&end);
        sec = end - start;
    }

    return 0;
}
