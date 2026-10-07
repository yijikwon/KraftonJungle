/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Marauders",
    /* First member's full name */
    "Sirius Black",
    /* First member's email address */
    "Sirius@pottermore.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define WSIZE 4 /* "워드(word)" 헤더 하나, 푸터 하나가 각각 4 byte(= 1 word)*/
#define DSIZE 8 /* "더블 워드(double word)", 워드 두개의 크기. - 헤더 + 푸터를 합친 크기 또는 블록 크기를 맞추는 단위 (블록은 항상 8의 배수)*/
#define CHUNKSIZE (1<<12) /* 힙이 모자라면 운영체제에 메모리를 더 달라고 함. 그때 한 번에 얼마씩 늘릴지를 정하는 값. 조금씩 자주 요청하면 느려지기 때문에 4096 byte씩 받음. */

#define GET(p) (* (unsigned int *)(p))
/*
    하는 일 : p 주소에서 4 byte를 숫자 하나로 읽어온다.
    왜 필요한가 : 헤더 / 푸터가 4 byte짜리 숫자라서, 헤더에 적힌 내용(블록 크기 + 사용 여부)을 꺼낼 때 쓴다.
    조각별 의미 : (p), 받은 주소 (덩어리가 들어와도 안전하게 괄호)
               (unsigned int *), 이 주소를 "4 byte짜리를 가리키는 주소"로 취급
               맨 앞 *, 그 주소를 찾아가서 값을 꺼냄
*/
#define PUT(p, val) (* ((unsigned int *)(p)) = (val))
/*
    하는 일 : p 주소에서 4 byte짜리 숫자 val을 써 넣는다
    왜 필요한가 : 헤더 / 푸터에 블록 정보를 기록할 때 쓴다
    GET과의 차이 : 생긴 건 GET과 같지만, 뒤에 = (val)만 붙음. 꺼내는 대신 써 넣는 것
*/

#define PACK(size, alloc) ((size)|(alloc))
/*
    하는 일 : 블록 크기와 사용 여부를 숫자 하나로 합친다
    왜 가능한가 :  크기는 항상 8의 배수라 2진수 끝자리가 비어 있어서, 거기에 사용 여부(0 | 1)을 끼워 넣을 수 있다.
    예시 : PACK(24, 1) = 25
*/

#define GET_ALLOC(p) (GET(p)&(0x1))
/*
    = #define GET_ALLOC(p) (*(unsigned int *)(p)&(0x1))
    하는 일 : 주소 p에 있는 헤더(또는 푸터)를 읽어서, 이 블록이 사용 중인지(1) 비어 있는지(0)만 꺼낸다
    왜 필요한가 : free할 때, 그리고 빈 블록을 찾거나 이웃 블록과 합칠 때 "이 블록 비어 있나?"르르 확인해야 해서
    어떻게 하나 : GET(p) - p 주소의 헤더 숫자를 읽어옴(예 : 25)
               & 0x1 - 맨 끝 자리만 남기고 나머지는 0으로 지움 -> 결과가 0 또는 1
         예시 : 해더가 25(11001)면 1(사용 중), 24(11000)면 0(비어있음)
    PACK과의 관계 : PACK이 끝 자리에 사용 여부를 넣었다면, GET_ALLOC은 그걸 다시 꺼낸다.
*/

#define GET_SIZE(p) (GET(p)&(~0x7))
/*
    하는 일 : 주소 p의 헤더(또는 푸터)를 읽어서, 사용 여부 자리를 지우고 블록 크기만 꺼낸다
    어떻게 하나 : GET(p) - 헤더 숫자를 읽어옴 (예 : 25)
               ~0x7 - 7(...00111)을 뒤집은 것 = 끝 3자리만 0인 마스크(...11000)
               & - 끝 3자리를 지우고 나머지는 그대로 남김
         예시 : 헤더가 25(11001)면 24(11000)
    GET_ALLOC과 짝 : GET_ALLOC은 끝 자리만 남기고, GET_SIZE는 끝 3자리를 지운다
    주의 : p는 헤더/푸터의 주소여야 한다 (bp 아님)
*/

#define HDRP(bp)((char *)(bp) - WSIZE)
/*
    하는 일 : bp(payload 시작 주소)를 받아서 그 블록의 헤더 주소를 돌려준다
    어떻게 : bp에서 WSIZE(4바이트)만큼 앞으로 간다
    왜 (char *) : 1바이트 단위로 정확히 4칸 이동하려고
    주의 : 결과는 주소이다. 헤더 값이 필요하면 GET(HDRP(bp)), 크기가 필요하면 GET_SIZE(HDRP(bp))
*/

#define FTRP(bp)((char *)bp + (GET_SIZE(HDRP(bp))225 -DSIZE)
/*
    하는 일 : bp(patload 시작 주소)를 받아서 그 블록의 푸터 주소를 돌려준다
    어떻게 하나 : GET_SIZE(HDRP(bp)) - 헤더에 적힌 블록 크기를 꺼냄(안쪽부터 : 헤더 주소 -> 크기)
               (char *)(bp) + 크기 - bp에서 블록 크기만큼 가면 다름 블록의 bp에 도착하 (너무 감)
               - DSIZE - 다음 블록 헤더(4) + 우리 푸터 (4) = 8칸 되돌아오면 푸터 시작
    예시 : bp = 108, 헤더 값 25(크기 24) -> 108 + 24 - 8= 124
    HDRP와 차이 : 헤더는 bp 바로 앞이라 크기와 상관없이 항상 bp - 4. 푸터는 블록 끝에 있어서 크기를 알아야 위치를 구할 수 있다.
    주의 : 헤더 값을 크기로 그대로 쓰면 안 됨(사용 여부 비트가 섞여 있음). 꼭 GET_SIZE로 걸러야함
          헤더에 크기를 먼저 써둔 뒤에만 쓸 수 있음. 헤더가 비어 있으면 엉뚱한 곳을 가리킴
*/

#define NEXT_BLKP(bp)((char *)(bp) + GET_SIZE(HDRP(bp)))
/*
    하는 일 : bp를 받아서 바로 다음 블록의 bp를 돌려준다
    어떻게 : 헤더에서 내 블록 크기를 읽고, bp에서 그만큼 앞으로 간다
    FTRP와 관계 : FTRP는 녀기서 DSIZE만큼 되돌아온 것
    예시 : bp = 108, 크기 24 -> 132
*/

#define PREV_BLKP(bp)((char *)(bp) - GET_SIZE((char *)(bp) - (DSIZE)))
/*
    하는 일 : bp를 받아서 바로 이전 블록의 bp를 돌려준다
    어떻게 : (char *)(bp) - DSIZE - 내 헤더(4) + 이전 푸터(4)만큼 뒤로 -> 이전 블록의 푸터 주소
            GET_SIZE(...) - 이전 푸터에서 이전 블록 크기를 읽음
            bp - 이전 크기 - 그만큼 뒤로 가면 이전 블록의 bp
    예시 : bp = 108, 이전 크기 16 -> 108 - 16 = 92
    푸터가 필요한 이유 : 이전 블록 크기를 내 bp 바로 근처(8칸 뒤)에서 읽을 수 있게 해줌. 헤더만 있으면 뒤로 갈 방법이 없음
    NEXT_BLKP와 짝 : NEXT는 내 헤더에서 크기를 읽고 앞으로, PREV는 이전 푸터에서 크기를 읽고 뒤로
*/

// implicit 방식으로 우선 해보기
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define CHUNKSIZE (1<<12) /* 힙이 모자랄 때 한 번에 늘릴 크기*/

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














