"""
[재귀 함수 - 팩토리얼과 피보나치 수열]

문제 설명:
- 재귀 함수를 사용하여 팩토리얼과 피보나치 수를 계산합니다.
- 재귀의 기본 개념인 base case와 recursive case를 이해합니다.

입력:
- n: 양의 정수

출력:
- 팩토리얼: n!
- 피보나치: n번째 피보나치 수

예제:
입력: n = 5
팩토리얼 출력: 120 (5! = 5 × 4 × 3 × 2 × 1)
피보나치 출력: 5 (0, 1, 1, 2, 3, 5)

힌트:
- 팩토리얼: n! = n × (n-1)!, 0! = 1
- 피보나치: fib(n) = fib(n-1) + fib(n-2), fib(0) = 0, fib(1) = 1
"""

def factorial(n):
    """
    재귀를 사용한 팩토리얼 계산
    
    Args:
        n: 양의 정수
    
    Returns:
        n의 팩토리얼 값
    """
    # TODO: base case를 작성하세요 // 기저 조건 
    # n이 0이거나 1이면 1을 반환

    # / base case (기저 조건) : "언제 멈출 것인가." 더 이상 쪼갤 수 없는, 답이 이미 정해져 있는 가장 간단한 경우. 재귀 호출을 하지 않고 바로 값을 리턴
    # / base case가 없다면 자기 자신을 무한히 호출해서 RecursionError가 남.

    if n == 0 or n ==1:
        return 1

    # TODO: recursive case를 작성하세요 // 재귀 조건

    # / Recursive case (재귀 조건) : "어떻게 더 작은 문제로 쪼갤 것인가" 더 작은 같은 종류의 문제 + 약간의 추가 작업으로 표현하는 부분.
    # / 함수가 자기 자신을 다시 호출함.(단, 더 작은 입력으로. 그래야 언젠가 base case에 도달함.)

    else:
        return n * factorial(n-1)

def fibonacci(n):
    """
    재귀를 사용한 피보나치 수 계산
    
    Args:
        n: 구하고자 하는 피보나치 수의 인덱스
    
    Returns:
        n번째 피보나치 수
    """
    # TODO: base case를 작성하세요
    # n이 0이면 0, n이 1이면 1 반환
    
    if n == 0:
        return 0
    elif n == 1:
        return 1
    
    # TODO: recursive case를 작성하세요

    else: 
        return fibonacci(n-1) + fibonacci(n-2)


# 테스트 케이스
if __name__ == "__main__":
    # 팩토리얼 테스트
    print("=== 팩토리얼 계산 ===")
    for i in range(6):
        result = factorial(i)
        print(f"{i}! = {result}")
    print()
    
    # 피보나치 테스트
    print("=== 피보나치 수열 ===")
    for i in range(10):
        result = fibonacci(i)
        print(f"fib({i}) = {result}")
    print()
    
    # 추가 테스트
    print("=== 추가 테스트 ===")
    print(f"10! = {factorial(10)}")
    print(f"fib(15) = {fibonacci(15)}")


