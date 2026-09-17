"""
[완전 탐색 - 배열에서 두 수의 합 찾기]

문제 설명:
- 정수 배열과 목표 값이 주어졌을 때, 배열에서 두 수를 선택하여 
  그 합이 목표 값과 같아지는 모든 쌍을 찾습니다.
- 완전 탐색(Brute Force) 방식으로 모든 경우를 확인합니다.

입력:
- nums: 정수 배열
- target: 목표 합

출력:
- 합이 target이 되는 (i, j) 인덱스 쌍의 리스트 (i < j)

예제:
입력: nums = [2, 7, 11, 15, 3], target = 9
출력: [(0, 1), (0, 4)]
설명: nums[0] + nums[1] = 2 + 7 = 9
      nums[0] + nums[4] = 2 + 7 = 9 (중복이지만 인덱스가 다름)

실제로는: nums[0] + nums[1] = 2 + 7 = 9만 해당

힌트:
- 이중 반복문을 사용하여 모든 쌍을 확인하세요
- i < j 조건을 유지하여 중복을 방지하세요
"""

 # 완전 탐색(Brute Force) 방식 : 모든 경우릐 수를 하나도 빠짐없이 다 확인해보는 방법.
 # 왜 이중 반복문이 필요할까?
 # 배열 안에서 두개를 골라서 더했을 때 target이 되는 조합을 전부 찾아야함. 
 # 외부 반목문에서 첫 번째로 고를 인덱스 (i)를 하나씩 돌고, 내부 반목분에서 두 번째로 고를 인덱스 (j)를 돌게해야함.

def find_two_sum_pairs(nums, target):
    """
    배열에서 합이 target이 되는 모든 인덱스 쌍 찾기
    
    Args:
        nums: 정수 배열
        target: 목표 합
    
    Returns:
        list: (i, j) 인덱스 쌍의 리스트
    """
    pairs = [] # 결과를 모아둘 빈 리스트.
    n = len(nums) # 배열의 원소 개수를 구하는 것
    
    # TODO: 이중 반복문으로 모든 쌍을 확인하세요
    ## 외부 반복문: i는 0부터 n-1까지 // 왜냐면 인덱스 번호는 시작 0부터 시작하니까 마지막 번호는 n-1이 됨.
    ## 내부 반복문: j는 i+1부터 n까지 (중복 방지)
    ## nums[i] + nums[j]가 target과 같으면 (i, j)를 결과에 추가

    # 중첩 반복문(nested loop) : 반복문 안에 반복문.
    for i in range(n): # range(n)는 0부터 n-1까지의 숫자를 순서대로 만듦. 반복문이 돌 때마다 i에 숫자 하나씩 들어옴. 
                       # i는 쌍의 첫 번째로 고를 원소 인덱스(번호로 일단 생각하기. 나중에 더할 때 값을 더하면됨. 그래서 하단 i + 1 은 인덱스 다음 번호를 말함.)
            for j in range(i + 1, n): # i +1로 시작해서 n 직전(n-1)까지 숫자를 만듦. j는 쌍의 두 번째로 고를 원소의 인덱스 인데, 
                                      # 항상 i 보다 큰 값부터 시작해서(i + 1)자기 자신과 짝짓거나 같은 쌍이 중복으로 나오는 걸 막는다. 
                if nums[i] + nums[j] == target: # 요구하는 target같이 맞다면
                    pairs.append((i,j)) # pairs에 추가?함.append()안에 ()가 있는 건 튜플로 만들어서 돌려주는 것.
    
    
    return pairs

# 테스트 케이스
if __name__ == "__main__":
    # 테스트 케이스 1
    nums1 = [2, 7, 11, 15]
    target1 = 9
    result1 = find_two_sum_pairs(nums1, target1)
    print(f"배열: {nums1}")
    print(f"목표 합: {target1}")
    print(f"결과 쌍: {result1}")
    print()
    
    # 테스트 케이스 2
    nums2 = [1, 3, 4, 2, 5, 6]
    target2 = 7
    result2 = find_two_sum_pairs(nums2, target2)
    print(f"배열: {nums2}")
    print(f"목표 합: {target2}")
    print(f"결과 쌍: {result2}")
    print()
    
    # 테스트 케이스 3
    nums3 = [1, 1, 1, 1]
    target3 = 2
    result3 = find_two_sum_pairs(nums3, target3)
    print(f"배열: {nums3}")
    print(f"목표 합: {target3}")
    print(f"결과 쌍: {result3}")


