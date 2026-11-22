#include "bptree.h"
#include <stdio.h>
#include <stdlib.h>

// ***************************************************
//  B+ Tree 구현에 필요한 함수들을 정의.
// ***************************************************

/*
    * 새로운 노드를 생성하는 함수.
    * is_leaf: 노드가 리프 노드인지 여부를 나타내는 플래그.
    * 반환값: 생성된 노드의 포인터.
*/

Node* create_node(bool is_leaf) 
{
    Node* new_node = (Node*)malloc(sizeof(Node));

    if (!new_node) 
    {
        printf("Memory allocation failed for new node.\n");
        exit(1);
    }

    new_node->keys_count = 0;
    new_node->is_leaf = is_leaf;
    new_node->next = NULL;
    new_node->parent = NULL;

    for (int i = 0; i < MAX_CHILDREN + 1; i++) 
    {
        new_node->children[i] = NULL;
    }
    
    return new_node;
}
/*
    < bptree_splitChild 함수 설명 >

1. mid = (key_count + 1) / 2로 설정 (왼쪽 편향)
2. 오른쪽 새 노드 생성 (is_leaf = child->is_leaf)
3. keys[mid .. n-1]과 values[mid .. n-1]을 오른쪽 노드로 복사
4. 개수 갱신: left->key_count = mid, right->key_count = n - mid
5. 리프 연결 갱신: right->next = left->next → left->next = right (순서 중요)
6. 부모 포인터를 쓴다면 right->parent = left->parent
7. 올라갈 키 = right->keys[0] (복사본, 오른쪽에도 남음)를 직접 설정

*/

void bptree_splitChild(Node* parent, int index) 
{
    Node* child = parent->children[index];
    Node* new_child = create_node(child->is_leaf);
    int mid = 0;
    int up_key = 0;

    // 자식 노드가 리프 노드인지 내부 노드인지에 따라 분할 방식이 다름
    if(child->is_leaf) 
    {
        //왼쪽 편향 : 왼쪽으로 가져가야하는 부분이 많음
        mid = (child->keys_count + 1) / 2;
        up_key = child->keys[mid];

        // 리프 노드인 경우, mid부터 끝까지의 키를 새 노드로 이동
        for (int i = mid; i < child->keys_count; i++) 
        {
            new_child->keys[i - mid] = child->keys[i];
        }
        new_child->keys_count = child->keys_count - mid;
        child->keys_count = mid; // 왼쪽 노드의 키 개수 갱신
    } 
    else  // 내부 노드인 경우
    {
        //왼쪽 편향 : 왼쪽으로 가져가야하는 부분이 많음
        mid = (child->keys_count) / 2;
        up_key = child->keys[mid];

        // 내부 노드인 경우, mid+1부터 끝까지의 키와 자식을 새 노드로 이동
        for (int i = mid + 1; i < child->keys_count; i++) 
        {
            new_child->keys[i - (mid + 1)] = child->keys[i];
        }

        for (int i = mid + 1; i <= child->keys_count; i++) 
        {
            new_child->children[i - (mid + 1)] = child->children[i];
            if (new_child->children[i - (mid + 1)]) 
            {
                new_child->children[i - (mid + 1)]->parent = new_child;
            }
        }

        new_child->keys_count = child->keys_count - (mid + 1);
        child->keys_count = mid; // 왼쪽 노드의 키 개수 갱신
    }


    // 리프 노드 연결 갱신
    if (child->is_leaf) 
    {
        new_child->next = child->next;
        child->next = new_child;
    }

    // 부모 노드에 새 자식 추가
    for (int i = parent->keys_count; i > index; i--) 
    {
        // 부모 노드의 자식 포인터와 키를 오른쪽으로 이동
        parent->children[i + 1] = parent->children[i];
        parent->keys[i] = parent->keys[i - 1];
    }

    parent->children[index + 1] = new_child;
    parent->keys[index] = up_key; // 올라갈 키 설정
    parent->keys_count++;
}


/*
    <bptree_insertNonFull 함수 설명>
1. 현재 노드가 리프 노드인지 확인
2. 리프 노드인 경우:
   - 키를 삽입할 위치를 찾음
   - 키를 삽입하고 keys_count 증가

3. 리프 노드가 아닌 경우:       
   - 적절한 자식 노드를 찾음
   - 자식 노드가 포화 상태인 경우 분할
   - 분할 후 적절한 자식 노드를 선택
   - 재귀적으로 bptree_insertNonFull 호출


*/

void bptree_insertNonFull(Node** root, int key) 
{
    Node* current = *root;

    // 리프 노드인 경우
    if (current->is_leaf) 
    {
        int i = current->keys_count - 1;

        // 키를 삽입할 위치를 찾음
        while (i >= 0 && current->keys[i] > key) 
        {
            current->keys[i + 1] = current->keys[i];
            i--;
        }

        // 키를 삽입
        current->keys[i + 1] = key;
        current->keys_count++;
    } 
    else // 내부 노드인 경우 
    {
        // 자식 노드 중 적절한 위치를 찾음
        int i = current->keys_count - 1;
        while (i >= 0 && current->keys[i] > key) 
        {
            i--;
        }
        i++;

        // 자식 노드가 포화 상태인 경우 분할
        if (current->children[i]->keys_count == MAX_KEY_SIZE) 
        {
            bptree_splitChild(current, i);

            // 분할 후 적절한 자식 노드를 선택
            if (key >= current->keys[i]) 
            {
                i++;
            }
        }

        bptree_insertNonFull(&(current->children[i]), key);
    }
}

/*
    * B+ Tree에 키를 삽입하는 함수.
    * root: 루트 노드의 포인터에 대한 포인터.
    * key: 삽입할 키 값.
    * 반환값: 삽입 성공 여부 (0: 성공, -1: 실패).

*/

int bptree_insert(Node** root, int key) 
{
    if(root == NULL)
    {
        printf("Error: root is NULL.\n");
        return -1;
    }
    
    if(*root == NULL)
    {
        *root = create_node(true);
        (*root)->keys[0] = key;
        (*root)->keys_count = 1;

        return 0;
    }

    //root가 포화 상태인 경우
    if((*root)->keys_count == MAX_KEY_SIZE)
    {
        Node* newRoot = create_node(false);
        newRoot->children[0] = *root;
        bptree_splitChild(newRoot, 0);
        *root = newRoot;
    }

    bptree_insertNonFull(root, key);


    return 0;
}

/*
    * B+ Tree를 레벨별로 출력하는 함수.
    * root: 출력할 트리의 루트 노드.
    * level: 현재 노드의 레벨 (루트는 0).
*/
void bptree_print(Node* root, int level) 
{
    if (root == NULL) 
    {
        return;
    }

    // 현재 노드의 키들을 출력
    printf("Level %d: ", level);
    for (int i = 0; i < root->keys_count; i++) 
    {
        printf("%d ", root->keys[i]);
    }
    printf("\n");

    // 자식 노드들을 재귀적으로 출력
    if (!root->is_leaf) 
    {
        for (int i = 0; i <= root->keys_count; i++) 
        {
            bptree_print(root->children[i], level + 1);
        }
    }
}


/*
    B+ Tree의 리프 노드들을 순차적으로 출력하는 함수.
    root: 출력할 트리의 루트 노드.
*/

void bptree_leaf_print(Node* root) 
{
    if (root == NULL) 
    {
        return;
    }

    // 리프 노드까지 내려감
    while (!root->is_leaf) 
    {
        root = root->children[0];
    }

    // 리프 노드들을 순차적으로 출력
    while (root != NULL) 
    {
        for (int i = 0; i < root->keys_count; i++) 
        {
            printf("%d ", root->keys[i]);
        }
        root = root->next;
    }
    printf("\n");
}


void bptree_destroy(Node* root) 
{
    if (root == NULL) 
    {
        return;
    }

    // 자식 노드들을 재귀적으로 해제
    if (!root->is_leaf) 
    {
        for (int i = 0; i <= root->keys_count; i++) 
        {
            bptree_destroy(root->children[i]);
        }
    }

    // 현재 노드 해제
    free(root);
}

