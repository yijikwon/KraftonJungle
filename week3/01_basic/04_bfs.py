"""
[BFS - 너비 우선 탐색 (Breadth-First Search)]

문제 설명:
- BFS로 그래프를 탐색합니다.
- 가까운 정점부터 방문합니다.
- 큐(Queue)를 사용합니다.

입력:
- graph: 그래프 (인접 리스트)
- start: 시작 정점

출력:
- 방문 순서

예제:
그래프:
  0 ─── 1
  │     │
  └─ 2 ─┘
      │
      3

시작: 0
BFS: [0, 1, 2, 3]

힌트:
- Week2의 큐 사용
- 방문 체크 필요
- 가까운 것부터 방문
"""

from collections import deque

def bfs(graph, start):
    """
    너비 우선 탐색
    
    Args:
        graph: 그래프 딕셔너리
        start: 시작 정점
    
    Returns:
        방문 순서 리스트
    """
    visited = [] # / 방문 순서를 기록할 리스트(최종 결과)
    
    # TODO: 큐 생성 및 시작 정점 추가
    ## 방문한 정점 집합
    queue = deque([start]) # / 뭔가를 넣어서 큐를 초기 상태로 만든 것. # 큐 생성, start 하나만 넣고 시작.
    visited.append(start) # / start는 시작 정점을 미리 방문 표시해두는 것. (중복 방지)
                          # / 왜 미리 표시하는가? BFS를 진행 하다보면 얘가 큐에 들어갔던 애인가? 확인해야함. 안그러면 중복처리 되거나, 무한 반복에 들어감.

    # print(queue)
    # print(visited)
    
    
    # TODO: 큐가 빌 때까지 반복
    ## 큐에서 정점 꺼내기
    ## 인접한 정점들 확인
    ## 방문하지 않은 정점이면 큐에 추가
    while queue: # for 처럼 정해진 횟수가 아니라, 조건이 참인 동안 계속 반복하는 거. 큐가 비어있지 않은 동안 반복해야하니까... # 큐가 텅빌 때 까지 반복.
                 # 파이썬에서는 리스트나 큐 같은게 비어있으면 False, 뭔가가 들어있으면 True로 취급됨.
        current = queue.popleft() # 맨 앞(가장 먼저 넣은 것)을 꺼냄
        for neighbor in graph[current]: # current의 이웃들을 하나씩 확인
            if neighbor not in visited: # 아직 방문 안(큐에 안 넣은) 이웃이면
                visited.append(neighbor) # 방문 표시
                queue.append(neighbor) # 나중에 처리하도록 큐 뒤에 추가.
    
    return visited # 방문했던 순서를 그대로 반환

# 테스트 케이스
if __name__ == "__main__":
    # 그래프 생성
    graph = {
        0: [1, 2],
        1: [0, 2],
        2: [0, 1, 3],
        3: [2]
    }
    
    print("=== BFS (너비 우선 탐색) ===")
    result = bfs(graph, 0)
    print(f"시작 정점: 0")
    print(f"방문 순서: {result}")

'''  
개인 이해를 위한 풀이.

직접 실행하면,

[1단계] 큐에서 꺼냄: 0 (남은 queue = [])
    -> 이웃 1 : 아직 방문 안 함 -> visited에 추가, queue에 추가.
    -> 이웃 2 : 아직 방문 안 함 -> visited에 추가, queue에 추가.
    현재 상태 : queue = [1, 2], visited = [0, 1, 2]
[2단계] 큐에서 꺼냄 : 1 (남은 queue = [2])
    -> 이웃 0 : 이미 방문했음 -> 건너뜀
    -> 이웃 2 : 이미 방문했음 -> 건너뜀
    현재 상태 : queue = [2], visited = [0, 1, 2]
[3단계] 큐에서 꺼냄 : 2 (남은 queue = [])
    -> 이웃 0 : 이미 방문했음 -> 건너뜀
    -> 이웃 1 : 이미 방문했음 -> 건너뜀
    -> 이웃 3: 아직 방문 안 함 -> visited에 추가, queue에 추가.
    현재 상태 : queue = [3], visited = [0, 1, 2, 3]
[4단계] 큐에서 꺼냄 : 3 (남은 queue = [])
    -> 이웃 2 : 이미 방문했음 -> 건너뜀
    현재 상태 : queue = [], visited = [0, 1, 2, 3]

최종 방문 순서 : [0, 1, 2, 3]
'''