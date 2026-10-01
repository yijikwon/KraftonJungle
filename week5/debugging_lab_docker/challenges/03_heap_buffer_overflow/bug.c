/*
 * Challenge 03 — Heap Buffer Overflow (심화: 동적 배열 성장 버그)
 *
 * [시나리오]
 *   자동 성장하는 정수 동적 배열 IntList (init/ensure/push/sum). 용량이 부족하면
 *   list_ensure() 가 용량을 2배로 늘리고 realloc 한다. 이 리스트로 큰 수열을
 *   만들어 합을 구한다.
 *
 * [기대 동작]
 *   0..N-1 을 100 으로 나눈 나머지를 리스트에 넣고, 길이·용량·합을 출력한 뒤 정상 종료.
 *
 * [증상]
 *   list_ensure() 가 새 용량(newcap)을 계산해 l->cap 에는 반영하지만,
 *   정작 realloc 은 "옛 용량(l->cap)" 으로 호출한다. 즉 논리 용량(cap)은 커지는데
 *   실제 버퍼는 한 세대 뒤처져, push 가 실제 버퍼 밖으로 계속 쓴다.
 *   힙 경계를 넘어 쓰면서 힙 메타데이터가 깨지거나(→ 이후 realloc/free 에서 SIGABRT)
 *   매핑되지 않은 페이지까지 밀고 나가 SIGSEGV. 크래시는 push 의 대입 지점 또는
 *   다음 realloc 에서 나지만, 원인은 ensure 의 realloc 인자다.
 *
 * [gdb 로 잡기]
 *   make gdb NAME=03_heap_buffer_overflow
 *   (gdb) run                         → 크래시(SIGSEGV) 또는 abort
 *   (gdb) bt                          → list_push 의 l->data[l->len]=x 또는 realloc 내부
 *   (gdb) frame N ; print *l           → cap 은 큰데 실제 버퍼는 그보다 작음(불일치)
 *   (gdb) print l->len  / print l->cap → len 이 실제 확보량을 넘어섰는지 확인
 *   (gdb) break list_ensure           → newcap 과 realloc 에 넘기는 크기를 대조
 *
 * [printf(로그)로 잡기]
 *   ensure 에서 (old cap, newcap, realloc 에 넘기는 크기) 를 함께 찍어 불일치를 본다:
 *     fprintf(stderr, "ensure old=%zu new=%zu realloc_bytes=%zu\n",
 *             l->cap, newcap, l->cap * sizeof(int));
 *   → newcap 과 realloc 크기가 다르면 그게 원인.
 *   (stdout 은 버퍼링되니 stderr 로 찍어야 크래시 직전 로그가 남는다)
 *
 * TODO: realloc 은 반드시 "새 용량(newcap)" 으로 호출하고, l->cap 갱신과 순서를 맞춰야 한다.
 *       (성장 로직은 '용량 필드'와 '실제 확보량'이 항상 같도록 유지해야 한다)
 */
#include <stdio.h> // "표준 입출력" 기능 모음을 가져옴
#include <stdlib.h> // "표준 유틸리티" 기능 모음, 메모리 / 프로세스 관련 함수를 쓰려면 필요.

typedef struct {
    int   *data;
    // data는 "정수들이 쭉 나열된 배열의 시작 위치를 가리키는 포인터"
    // 지금 이 시점엔 아직 어떤 배열도 안 만들어졌으니, 나중에 채워질 자리 

    /* [Thinking Point]
     * 개수/크기를 담는 len, cap 을 왜 int 가 아니라 size_t 로 선언할까?
     *   tip 1. size_t 는 "이 플랫폼에서 표현 가능한 가장 큰 객체 크기"를 담도록 만든
     *          부호 없는(unsigned) 정수 타입이다. malloc/sizeof/strlen 의 타입도 size_t 다.
     *   tip 2. int 는 보통 32비트라 약 21억(2^31-1)에서 넘치고, 음수도 가능하다.
     *          원소가 그보다 많아지거나 cap*sizeof(int) 계산이 커지면 int 는 오버플로된다.
     *   생각해보기: 크기를 int 로 두면 어떤 버그가 생길 수 있을까?
     */
    size_t len;
    // len(lenght, 길이) : 지금까지 실제로 채워 넣은 원소의 개수.
    // 예) 원소 3개 넣었으면 len = 3
    size_t cap;
    // cap(capacity, 용량) : 지금 확보해둔 "수납공간"이 총 몇 칸인지.
    // len과 다른 개념! cap = 8 이면 "8칸을 마련해뒀다"라는 뜻이고,
    // 그중 실제로 몇 칸이 채워졌는지는 len이 알려줌.
    // 항상 len <= cap 이어야 정상임. (채운 개수가 마련한 칸보다 많을 수 없음)

} IntList;
// typeof...IntList; -> 위에서 정의한 이 묶음에 "IntList" 라는 이름을 붙임.
// 이제부터 IntList 라는 이름 하나로 "포인터 + len + cap 세트"를 통째로 다룰 수 있음.



static void list_init(IntList *l) {
// static : 이 함수는 이 파일 안에서만 쓰겠다는 표시
// void : 이 함수는 결과값을 돌려주지 않는다는 뜻
// IntList * l : "IntList 하나를 가리키는 포인터"를 매개변수로 받음.
//               -> 왜 "IntList 자체가 아니라 왜 포인터로 받을까?" 
//               함수 안에서 l -> cap, l -> len, l -> data를 실제로 "변경"해야 하는데,
//               만약 포인터가 아니라 원본 복사본을 받으면, 함수 안에서 값을 바꿔도
//               원래 main()에 있는 진짜 IntList는 전혀 바뀌지 않습니다(복사본만 바뀜).
//               포인터로 받으면 "진짜 원본이 있는 위치"를 알고 있으니, 그 위치로 찾아가서
//               원본 자체를 직접 고칠 수 있게 되는 것.
    l->cap  = 8;
    // l -> cap : "l이 가리키는 그 IntList 구조체 안의 cap 필드"에 접근한다는 뜻
    //            (구조체를 포인커로 가지고 있을 때는 점(.)이 아니라 화살표(->)를 쓴다.
    //             만약 l이 포인터가 아니라 구조체 자체였다면 l.cap 이라고 썼을 것임.)
    // 여기서는 "일단 8칸짜리 창고를 마련하자"고 계획을 세우는 것.
    l->len  = 0;
    // 아직 하나도 안채웠으니 len은 0으로 시작.
    l->data = malloc(l->cap * sizeof(int));
    // malloc(...) : "힙(heap)"이라는 메모리 공간에서 괄호 안에 적은 바이트(byte) 수만큼
    //                빈 공간을 실제로 빌려오고, 그 공간이 시작하는 주소(쪽지)를 돌려주는 함수
    //                이 반환된 주소를 data 포인터에 저장하는 것.
    // sizeof(int) : "정수 하나가 메모리에서 몇 바이트를 차지하는지" (보통 4byte)
    // l -> cap * sizeof(int) : "8칸 * 한 칸당 4byte = 32byte"를 계산.
    //                           즉 "정수 8개를 담을 수 있는 딱 그만큼의 공간을 주세요"라는 요청
    // -> 결과적으로 이 줄은 "정수 8개짜리 배열을 담을 32바이트 공간을 새로 빌려서,
    //    그 시작 주소를 l -> data에 저장해라: 라는 뜻.
    if (!l->data) { perror("malloc"); exit(1); }
    // malloc은 만약 빌려줄 공간이 없으면(창고가 꽉 찼으면) 실패하고,
    // 그 표시로 NULL(아무 곳도 가리키지 않는 특수한 값, 사실상 0)을 돌려준다.
    // !l -> data : "l -> data가 NULL이면 (=malloc이 실패했으면)" 이라는 조건.
    // perror("malloc") : "malloc 함수에서 에러가 났다"는 메세지를 
    //                     화면(정확히는 표준 에러)에 출력해주는 함수.
    // exit(1) : 프로그램 즉시 강제 종료. 괄호 안 1은 관례적으로
    //           "정상 종료(0)가 아니라 뭔가 문제가 있어서 끝난다"는 표시
} // -> list_init는 "정수 8개를 담을 진짜 메모리 공간을 heap에서 가져오고, 그 위치를 data에 적어두고, 
  //    지금은 아직 아무것도 안 채웠다(len = 0)고 기록하는" 초기화 함수이다.

static void list_ensure(IntList *l, size_t need) {
// static : 이 파일 안에서만 쓰는 함수라는 표시
// void : 이 함수는 결과값을 따로 돌려주지 않는다는 뜻
// IntList * l : "IntList 진짜 상자의 주소가 적힌 쪽지"를 받음
//               (이 함수 안에서 원본 l -> cap, l -> data를 직접 고치기 위해 포이너로 받는 것)
// size_t need : 포인터 아님, 그냥 숫자 하나. "최소 이만큼의 자리가 확보되어
//               있어야 한다"는 요구치를 전달받는 매개변수
    if (need <= l->cap) return;
    // <= : "이하"를 뜻하는 비교 연산자. 참(true) / 거짓(false)을 판단.
    // need <= l -> cap : "필요한 개수가, 지금 이미 확보된 용량보다 작거나 같다면"
    //                     = "이미 자리가 충분하다면"
    // return; : 함수를 여기서 즉시 끝내고 빠져나가라는 명령.(이 아래 코드는 실행되지 않고 함수 호출이 끝남.)
    // -> 한줄 요약: "자리가 이미 충분하면 아무것도 하지 말고 그냥 끝내라."

    size_t newcap = l->cap ? l->cap * 2 : 8;
    // size_t newcap : 이 함수 안에서만 쓰는 새 변수. "이번에 목표로 할 새 용량"을 담을 그릇.
    // 조건? A : B : 삼항 연산자(ternary operator). if문을 한 줄로 압축한 문법.
    //              "조건이 참이면 전체 값은 A, 거짓이면 전체 값은 B가 된다."
    // l -> cap : 여기선 조건 자리에 옴. C언어는 0이 아닌 값을 "참", 0을 "거짓"으로 취금
    //            -> "l -> cap이 0이 아니면(이미 용량이 있다면)"이 참.
    // l -> cap * 2 : 조건이 참일 때 쓰는 값. "현재 용량의 2배"
    // 8 : 조건이 거짓일 때 (용량이 아직 0일 때)쓰는 값. 처음 시작 크기
    //-> 우리 예시(l -> cap = 8인 상태)로 계산하면 : 8은 0이 아니므로 참 -> newcap = 8 * 2 = 16
    while (newcap < need) newcap *= 2;
    // while (조건) 문장; : "조건이 참인 동안 그 문장을 계속 반복 실행하라"는 반복문
    // newcap < need : "지금 계산한 newcap이 아직도 필요한 양(need)보다 작다면"
    // newcap *= 2; : newcap = newcap * 2 를 줄여 쓴 표현. newcap을 2배로 늘림
    // -> "한 번 2배로 늘렸는데도 부족하면, 충분해질 때까지 계속 2배씩 더 늘려라"는 안전장치
    // (우리 예시 : newcap = 16, need = 9 -> 16 < 9는 거짓이라 이 줄은 한번도 실행 안 되고 통과)
    // int *p = realloc(l->data, l->cap * sizeof(int)); // 수정 전(오류)
    int *p = realloc(l -> data, newcap * sizeof(int));
    // int *p: "정수를 가리키는 포인터" 타입의 새 지역 변수 p
    //          (l -> data와 같은 종류의 쪽지. 여기 realloc의 결과 주소를 임시로 받아둠)
    // realloc(A, B): "A가 가리키던 기존 공간을, 총 B byte크기로 다시 조정해달라"는 함수.
    //                 malloc처럼 "새로 빌리는"게 아니라 "이미 빌린 걸 늘리거나 줄여달라"는 요청
    //                 성공하면 그 공간의 (어쩌면 새로운) 시작 주소를 돌려주고, 실패하면 NULL을 돌려줌.
    // l -> data : 첫 번째 인자. "지금 이 주소에 있는 기존 공간을 조정해줘"
    // ㅣ -> cap : * 두 번째 인자(원하는 총 바이트 수) 계산에 쓰이는 변수.
    //             주의 : 이 시점의 l -> cap은 "아직 갱신 전, 옛날 값"이다.
    //             (l -> cap이 새 값으로 바뀌는 건 이 함수 맨 마지막 줄에서 일어남.)
    // sizeof(int) : 정수 하나가 몇 바이트인지 (보통 4바이트)
    // l -> cap * sizeof(int) : "옛날 용량 개수 * 정수 한 개 바이트" = 옛날 기준 총 바이트 수
    // -> 이 줄 전체 뜻 : "realloc아, l -> data 공간을, '옛날 cap' 기준 바이트 수로 다시 조정해줘"
    //      (바로 위에서 새로 계산해둔 목표값 newcap이 아니라, 옛날 값 l -> cap을 쓰고 있다는 점을
    //       꼭 기억하기)
    if (!p) { perror("realloc"); free(l->data); exit(1); }
    // !p : "p가 NULL이면" = realloc이 실패했다면.
    // perror("realloc") : "realloc에서 실패했다"는 에러 메세지를 출력.
    // free(; -> data) : realloc이 실패해도 원래 있던 l -> data(옛날 공간)는 그대로 살아있으므로,
    //                   프로그램을 끝내기 전에 그 공간을 "반납"해주는 함수
    //                   (free는 malloc / realloc으로 빌린 공간을 다 쓰고 나서 창고에 돌려주는 함수.
    //                    안 돌려주면 그 공간은 프로그램이 끝날 때까지 계속 낭비됨 - "메모리 누수")
    // exit(1) : 비정상 종료.
    l->data = p;
    // l -> data = p; : realloc이 알려준 (어쩌면 새로운 위치의) 주소로 l -> data를 갱신
    //                  이제 원본 구조체의 data 포인터가 이 새 주소를 가리키게 됨.
    l->cap  = newcap;
    // l -> cap = newcap; : l -> cap 필드에, 아까ㅏ 계산해둔 목표 용량(newcap)을 저장.
    //                      이제부터 l -> cap은 "이만큼 확보되어 있다"고 다른 함수(list_push 등)에게 알려주는 값이 됨.
}

static void list_push(IntList *l, int x) {
// static :  이 파일 안에서만 쓰는 함수라는 표시
// void : 결과값을 따로 돌려주지 않는 함수
// IntList *1 : 지금까지 계속 봐온 것과 동일 - 원본 IntList 상자의 주소가 담긴 쪽지.
//              이 함수 안에서 l -> len, l -> cap, l -> data를 실제로 바꾸기 위해 포인터로 받음.
// int x : 포인터가 아니라 그냥 평범한 정수 값 하나.
//         (예 : main()에서 list_push(&l, i % 100)이라고 호출하니, x에서는 i % 100 값이 들어옴)
    if (l->len == l->cap) list_ensure(l, l->cap + 1);
    // == : "같다"를 검사하는 비교 연산자. (참고 :  =는 "대입"이고 ==는 "비교"라 다른 기호임.)
    // l -> len == l -> cap : "지금까지 채운 개수(len)와, 확보된 용량(cap)이 정확히 같아졌는가?"
    //                         같아졌다는 건 "더 이상 빈 자리가 없다"는 뜻
    // list_ensure(l, l 0> cap + 1) : 자리가 없다면, list_ensure 함수를 호출해서
    //                                "적어도 (지금 cap cap + 1)개 들어갈 자리를 만들어달라"고 요청.
    //                                 (l -> cap + 1 : 지금 cap보다 딱 1개 더 필요하다는 최소 요구치.
    //                                  list_ensure 안에서는 이 숫다를 보고 실제로는 2배씩 넉넉하게 늘려줌 
    // -> 이 줄 전체 뜻 : "자리가 꽉 찼으면, 미리 자리를 넓혀놓고 와라."(자리가 남아있으면 그냥 통과)
    l->data[l->len++] = x;
    // l -> data[...] : 대괄호 []는 "배열 인덱싱(array indexing)"문법
    //                  "l -< data가 가리키는 배열에서, []안의 번호 위치에 있는 칸"을 뜻함.
    //                  배열 번호(인덱스)는 0부터 시작합니다. (첫 번째 칸이 [0], 두 번째가 [1]...)
    // l -> len++ : "후위 증가 연산자(post-increment)". 두 가지 일을 순서대로 함
    //               ① 지금 이 자리에서는 먼저 l -> len의 "현재 값"을 그대로 사용하고,
    //               ② 그 다음에 l -> len 값을 1 증가시킨다.
    //              즉 "지금 len 값을 인덱스로 쓰고, 그 다음 len을 1 늘려라"는 뜻.
    //              (예 : len이 8이었다면 -> 이번에 인덱스 8번 칸에 접근하고, 그 후 len은 9가 됨)
    // l -> data[l -> len++] = x;
    //                      ->  전체 해석 : "l -> data 배열의 (현재 len번째) 칸에 x값을 저장하고, 
    //                                     저장을 마쳤으니 len을 1 늘려서 '이제 하나 더 채워졌다'고 기록해라.""
// -> 자리가 꽉 찼으면 먼저 list_ensure를 불러서 자리를 늘려놓고, 그 지금 채운 개수(len)를 인덱스로 삼아 그 칸에 새 값을 넣고, 채운 개수를 1 늘려라.
}

static long long list_sum(const IntList *l) {
    long long s = 0;
    for (size_t i = 0; i < l->len; i++) s += l->data[i];
    return s;
}

static void list_free(IntList *l) {
    free(l->data);
    l->data = NULL;
    l->len = l->cap = 0;
}

int main(void) {
    IntList l;
    list_init(&l);

    const int N = 2000000;
    for (int i = 0; i < N; i++) {
        list_push(&l, i % 100);        
    }

    printf("len=%zu cap=%zu sum=%lld\n", l.len, l.cap, list_sum(&l));
    list_free(&l);
    return 0;
}
