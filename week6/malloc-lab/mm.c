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

#define GET(p) * (unsigned int *)(p)
/*
    하는 일 : p 주소에서 4 byte를 숫자 하나로 읽어온다.
    왜 필요한가 : 헤더 / 푸터가 4 byte짜리 숫자라서, 헤더에 적힌 내용(블록 크기 + 사용 여부)을 꺼낼 때 쓴다.
    조각별 의미 : (p), 받은 주소 (덩어리가 들어와도 안전하게 괄호)
               (unsigned int *), 이 주소를 "4 byte짜리를 가리키는 주소"로 취급
               맨 앞 *, 그 주소를 찾아가서 값을 꺼냄
*/
#define PUT(p, val) * ((unsigned int *)(p)) = (val)
/*
    하는 일 : p 주소에서 4 byte짜리 숫자 val을 써 넣는다
    왜 필요한가 : 헤더 / 푸터에 블록 정보를 기록할 때 쓴다
    GET과의 차이 : 생긴 건 GET과 같지만, 뒤에 = (val)만 붙음. 꺼내는 대신 써 넣는 것
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














