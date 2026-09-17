"""
[문자열 - 회문(Palindrome) 판별]

문제 설명:
- 주어진 문자열이 회문(앞에서 읽으나 뒤에서 읽으나 같은 문자열)인지 판별합니다.
- 대소문자를 구분하지 않고, 공백과 특수문자는 무시합니다.

입력:
- s: 판별할 문자열

출력:
- True: 회문인 경우
- False: 회문이 아닌 경우

예제:
입력: "A man, a plan, a canal: Panama"
출력: True

입력: "race a car"
출력: False

힌트:
- 알파벳과 숫자만 남기고 소문자로 변환하세요
- 문자열을 뒤집어서 비교하거나, 양 끝에서 시작해 중앙으로 이동하며 비교하세요
"""

def is_palindrome(s):
    """
    문자열이 회문인지 판별하는 함수
    
    Args:
        s: 판별할 문자열
    
    Returns:
        bool: 회문이면 True, 아니면 False
    """
    
    # 정리한 글자를 저장할 빈 문자열을 만든다
    text = ""

    # 문자열을 한 글자씩 확인한다
    for char in s:
        # 알파벳이나 숫자인지 확인한다
        if char.isalnum(): 
        
        # isalnum()는 해당 문자열이 알파벳(문자)이나 숫자로만 이루어져 있는지 확인하는 메서드 
        # 공백, 특수만자, 기호 등 하나라도 섞여 있거나, 문자열이 비어있으면 False를 반환함.
        # 문자 또는 숫자만 있으면 True 반홤.

        # *** isaplha() 문자만 있는지 확인, isdigit() 숫자만 있는지 확인하는 메서드***
        
            # 소문자로 바꾼 후 text에 추가한다
            text = text + char.lower() 
            # Q. 왜 text = text + char.lower()인가
            # A. 반복문 안에서 왜 text = char.lower() 하면 반복될 때마다 기존의 값 위에 새로운 값이 씌어짐.
            #    그러면 내가 원하는 값이 도출되지 않기 때문에 기존의 값에 더해줘야하기 때문.

    # text를 뒤집는다
    reverse_text = text[::-1] # 파이썬의 슬라이싱(slicing 문법) 문자열[시작:끝:간격] -1이면 한칸씩 , 뒤에서 앞으로.

    # 두 문자열이 같은지 확인한다
    if text == reverse_text:
        return True # 앞뒤 동일 시 True
    else:
        return False # 불일치시 False
    
    # TODO: 알파벳과 숫자만 남기고 소문자로 변환하세요
    # 힌트: isalnum() 메서드와 lower() 메서드 사용
    pass
    
    # TODO: 정제된 문자열이 회문인지 확인하세요
    # 방법1: 문자열을 뒤집어서 비교 ([::-1] 사용)
    # 방법2: 양 끝 인덱스를 이용한 투 포인터 방식
    pass
    
    #return False

# 테스트 케이스
if __name__ == "__main__":
    # 테스트 케이스 1
    test1 = "A man, a plan, a canal: Panama"
    result1 = is_palindrome(test1)
    print(f"입력: \"{test1}\"")
    print(f"회문 여부: {result1}")
    print()
    
    # 테스트 케이스 2
    test2 = "race a car"
    result2 = is_palindrome(test2)
    print(f"입력: \"{test2}\"")
    print(f"회문 여부: {result2}")
    print()
    
    # 테스트 케이스 3
    test3 = "Was it a car or a cat I saw?"
    result3 = is_palindrome(test3)
    print(f"입력: \"{test3}\"")
    print(f"회문 여부: {result3}")
    print()
    
    # 테스트 케이스 4
    test4 = "Madam"
    result4 = is_palindrome(test4)
    print(f"입력: \"{test4}\"")
    print(f"회문 여부: {result4}")



