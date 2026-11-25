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

    leftNode가 리프 노드인 경우
 1-1. 새 노드 생성(mid = ((keys_count+1)/2))
 1-2. mid부터 keys_count까지 새노드에 저장
 1-3. 링크 설정 
    새 노드의 next = leftNode->next
    leftNode -> next = 새 노드
 1-4. parent 키와 자식을 뒤로 한칸씩 밀음 : for (키 : i = key_count-1 ~ pos, 자식 : for(i = key_count; i > pos; i--) )
    자식 parent->child[i+1] = parent->child[i];

 1-5. parent의 pos에 새노드의 0번째 키를 저장, parent->children[pos+1] = 새 노드
 1-6. parent와 left, 새노드의 키 갯수 설정
     리프: leftNode = mid, 새 노드 = cnt - mid, parent +1
     내부 노드: leftNode = mid, 새 노드 = cnt - mid - 1, parent +1

    leftNode가 리프가 아닌 경우
 2-1. 새 노드 생성 (mid = (keys_count/2), up_key = leftNode[mid]->keys)
 2-2. 새 노드에 mid+1부터 keys_count까지 키 저장
 2-3. 새 노드에 자식 저장 : mid+1부터 ket_count 이하
 2-4. parent 키와 자식을 뒤로 한칸씩 밀음 : for (키 : i = key_count-1 ~ pos, 자식 : for(i = key_count; i > pos; i--))
         자식 parent->child[i+1] =  parent->child[i];  
 2-5. parent의 pos에 up_key 넣고, 링크 설정 parent->children[pos+1] = 새 노드
 2-6. parent와 left, 새노드의 키 갯수 설정
     리프: leftNode = mid, 새 노드 = cnt - mid, parent +1
     내부 노드: leftNode = mid, 새 노드 = cnt - mid - 1, parent +1
*/

static void bptree_splitChild(Node* parent, int index) 
{
    Node * leftNode = parent->children[index];
    Node * newNode = create_node(leftNode->is_leaf);
    
    if(leftNode->is_leaf) // 리프 노드인 경우
    {
        int mid = (leftNode->keys_count + 1) / 2;

        // 새 노드에 mid부터 keys_count까지 키 저장
        for (int i = mid; i < leftNode->keys_count; i++) 
        {
            newNode->keys[i - mid] = leftNode->keys[i];
        }

        newNode->keys_count = leftNode->keys_count - mid;
        leftNode->keys_count = mid;

        // 링크 설정
        newNode->next = leftNode->next;
        leftNode->next = newNode;

        // 부모 노드에 키와 자식 포인터 삽입
        for (int i = parent->keys_count; i > index; i--) 
        {
            parent->keys[i] = parent->keys[i - 1];
            parent->children[i + 1] = parent->children[i];
        }

        parent->keys[index] = newNode->keys[0];
        parent->children[index + 1] = newNode;
        parent->keys_count++;
    } 
    else // 내부 노드인 경우
    {
        int mid = leftNode->keys_count / 2;
        int up_key = leftNode->keys[mid];

        // 새 노드에 mid+1부터 keys_count까지 키 저장
        for (int i = mid + 1; i < leftNode->keys_count; i++) 
        {
            newNode->keys[i - (mid + 1)] = leftNode->keys[i];
        }

        // 새 노드에 자식 저장
        for (int i = mid + 1; i <= leftNode->keys_count; i++) 
        {
            newNode->children[i - (mid + 1)] = leftNode->children[i];
            if (newNode->children[i - (mid + 1)]) 
            {
                newNode->children[i - (mid + 1)]->parent = newNode;
            }
            leftNode->children[i] = NULL;
        }

        newNode->keys_count = leftNode->keys_count - mid - 1;
        leftNode->keys_count = mid;

        // 부모 노드에 키와 자식 포인터 삽입
        for(int i = parent->keys_count; i > index; i--) 
        {
            parent->keys[i] = parent->keys[i - 1];
            parent->children[i + 1] = parent->children[i];
        }

        // 부모 노드의 키 및 링크 설정
        parent->keys[index] = up_key;
        parent->children[index+1] = newNode;
        ++parent->keys_count;
    }

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

static void bptree_insertNonFull(Node** root, int key) 
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
    * B+ Tree의 리프 노드들을 순차적으로 출력하는 함수.
    * root: 출력할 트리의 루트 노드.
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


/*
1. 양쪽 모두 여유가 없는 경우 → 병합
   - 오른쪽 형제와 병합 (마지막 자식이면 왼쪽 형제와)
   1-1. 리프
        - 오른쪽의 키를 왼쪽 뒤에 붙임 (부모 키는 내리지 않음)
        - left->next = right->next
   1-2. 내부 노드
        - 부모 키를 왼쪽 뒤에 내림
        - 오른쪽의 키와 자식을 왼쪽 뒤에 붙임
   1-3. 공통
        - 부모에서 그 키와 오른쪽 자식 포인터를 제거하고 당김
        - 오른쪽 노드 해제

*/

static void bptree_merge(Node* parent, int idx1, int idx2) 
{
    Node * left = parent->children[idx1];
    Node * right = parent->children[idx2];

    // 리프 노드인지 내부 노드인지에 따라 병합 방식이 다름
    if (left->is_leaf) 
    {
        // 리프 노드 병합
        for (int i = 0; i < right->keys_count; i++) 
        {
            left->keys[left->keys_count + i] = right->keys[i];
        }

        left->keys_count += right->keys_count;
        left->next = right->next;
    } 
    else 
    {
        // 내부 노드 병합
        left->keys[left->keys_count] = parent->keys[idx2 - 1];
        for (int i = 0; i < right->keys_count; i++) 
        {
            left->keys[left->keys_count + 1 + i] = right->keys[i];
        }
        for (int i = 0; i <= right->keys_count; i++) 
        {
            left->children[left->keys_count + 1 + i] = right->children[i];
            if (right->children[i]) 
            {
                right->children[i]->parent = left;
            }
        }
        left->keys_count += 1 + right->keys_count;
    }

    for(int i = idx1; i < parent->keys_count - 1; i++) 
    {
        parent->keys[i] = parent->keys[i + 1];
        parent->children[i + 1] = parent->children[i + 2];
    }


    parent->children[parent->keys_count] = NULL;
    --parent->keys_count;

    free(right);
}





/*
      < Remove >

1. 현재 노드가 리프이고 키가 있는 경우: 지우고 뒤의 키들을 앞으로 당깁니다.
2. 현재 노드가 리프이고 키가 없는 경우: 없음을 반환하고 종료합니다.
3. 현재 노드가 내부 노드인 경우
   3-1. idx = 지울 키보다 큰 첫 키의 위치
   3-2. child[idx]가 최소면 balance (리프냐 내부 노드냐에 따라 빌리기·병합 방식이 다름)
   3-3. 마지막 자식이 왼쪽과 병합됐으면 idx−1
   3-4. child[idx]로 재귀


*/

static void bptree_balance(Node* parent, int pos);

static int bptree_remove_helper(Node** root, int key)
{
    Node* current = *root;

    // 현재 노드가 리프 노드인 경우
    if (current->is_leaf) 
    {
        int idx = 0;

        while (idx < current->keys_count && current->keys[idx] < key) 
        {
            idx++;
        }

        // 키가 존재하지 않으면 종료
        if (idx == current->keys_count || current->keys[idx] != key) 
        {
            printf("Key %d not found in the tree.\n", key);
            return -1;
        }

        // 키를 삭제하고 뒤의 키들을 앞으로 당김
        for (int j = idx; j < current->keys_count - 1; j++) 
        {
            current->keys[j] = current->keys[j + 1];
        }
        --current->keys_count;
        return 0;
    }
    else // 현재노드가 내부 노드인 경우
    {
        int idx = 0;

        while (idx < current->keys_count && current->keys[idx] <= key) 
        {
            idx++;
        }

        // 자식 노드가 최소 키 개수인지 확인하고 필요시 balance 수행
        if (current->children[idx]->keys_count <= MIN_KEY_SIZE)
        {
            bptree_balance(current, idx);
            // 병합 후 idx가 변경될 수 있으므로 다시 확인
            if (idx > current->keys_count) 
            {
                idx--;
            }
        }

        return bptree_remove_helper(&(current->children[idx]), key);
    }
}


/*
< balance(parent, pos) >
1. 왼쪽 형제에 여유가 있는 경우 (pos > 0)
   1-1. 리프
        - 형제의 마지막 키를 내 맨 앞으로 이동
        - parent->keys[pos-1] = 내 새 첫 키
   1-2. 내부 노드 (회전)
        - parent->keys[pos-1]을 내 맨 앞으로 내림
        - 형제의 마지막 자식을 내 맨 앞 자식으로 이동
        - 형제의 마지막 키를 parent->keys[pos-1]로 올림

2. 오른쪽 형제에 여유가 있는 경우 (pos < parent->keys_count)
   2-1. 리프
        - 형제의 첫 키를 내 맨 뒤로 이동, 형제는 앞으로 당김
        - parent->keys[pos] = 형제의 새 첫 키
   2-2. 내부 노드 (회전)
        - parent->keys[pos]를 내 맨 뒤로 내림
        - 형제의 첫 자식을 내 맨 뒤 자식으로 이동
        - 형제의 첫 키를 parent->keys[pos]로 올림, 형제는 앞으로 당김


*/

static void bptree_borrowFromLeft(Node* parent, int pos) 
{
    Node * left_sibling = parent->children[pos-1];
    Node * current = parent->children[pos];


    if(current->is_leaf) // 리프 노드인 경우
    {
        // 왼쪽 형제의 마지막 키를 현재 노드의 맨 앞으로 이동
        for(int i = current->keys_count; i > 0; i--) 
        {
            current->keys[i] = current->keys[i-1];
        }
        current->keys[0] = left_sibling->keys[left_sibling->keys_count - 1];
        left_sibling->keys_count--;
        current->keys_count++;

        // 부모 키 갱신
        parent->keys[pos-1] = current->keys[0];
    } 
    else // 내부 노드인 경우
    {
        
        // 부모 키를 현재 노드의 맨 앞으로 이동
        for(int i = current->keys_count; i > 0; i--) 
        {
            current->keys[i] = current->keys[i-1];
        }
        current->keys[0] = parent->keys[pos-1];

        // 왼쪽 형제의 마지막 자식을 현재 노드의 맨 앞으로 이동
        for(int i = current->keys_count + 1; i > 0; i--) 
        {
            current->children[i] = current->children[i-1];
        }
        current->children[0] = left_sibling->children[left_sibling->keys_count];

        // 부모 키 갱신
        parent->keys[pos-1] = left_sibling->keys[left_sibling->keys_count - 1];

        left_sibling->children[left_sibling->keys_count] = NULL;
        
        left_sibling->keys_count--;
        current->keys_count++;
    }

}

static void bptree_borrowFromRight(Node* parent, int pos) 
{
    Node * right_sibling = parent->children[pos + 1];
    Node * current = parent->children[pos];

    if(current->is_leaf) // 리프 노드인 경우
    {
        // 오른쪽 형제의 첫 키를 현재 노드의 맨 뒤로 이동
        current->keys[current->keys_count] = right_sibling->keys[0];
        current->keys_count++;

        // 오른쪽 형제의 키들을 앞으로 이동
        for(int i = 0; i < right_sibling->keys_count - 1; i++) 
        {
            right_sibling->keys[i] = right_sibling->keys[i+1];
        }

        parent->keys[pos] = right_sibling->keys[0];
        right_sibling->keys_count--;

    } 
    else // 내부 노드인 경우
    {
        // 부모 키를 현재 노드의 맨 뒤로 이동
        current->keys[current->keys_count] = parent->keys[pos];
        current->keys_count++;

        // 오른쪽 형제의 첫 자식을 현재 노드의 맨 뒤 자식으로 이동
        current->children[current->keys_count] = right_sibling->children[0];

        // 오른쪽 형제의 자식들을 앞으로 이동
        for(int i = 0; i < right_sibling->keys_count; i++) 
        {
            right_sibling->children[i] = right_sibling->children[i+1];
        }
        right_sibling->children[right_sibling->keys_count] = NULL;

        // 부모 키 갱신
        parent->keys[pos] = right_sibling->keys[0];

        // 오른쪽 형제의 키들을 앞으로 이동
        for(int i = 0; i < right_sibling->keys_count - 1; i++) 
        {
            right_sibling->keys[i] = right_sibling->keys[i+1];
        }
        right_sibling->keys_count--;
    }

}

static void bptree_balance(Node* parent, int pos) 
{
    // 왼쪽 형제 노드가 존재하고 여유가 있는 경우
    if (pos > 0 && parent->children[pos - 1]->keys_count > MIN_KEY_SIZE)
    {
        // 왼쪽 형제로부터 키를 빌림
        bptree_borrowFromLeft(parent, pos);
            
    }
    // 오른쪽 형제 노드가 존재하고 여유가 있는 경우
    else if (pos < parent->keys_count && parent->children[pos + 1]->keys_count > MIN_KEY_SIZE) 
    {
        // 오른쪽 형제로부터 키를 빌림
        bptree_borrowFromRight(parent, pos);
        
    }
    else if(pos < parent->keys_count) // 양쪽 모두 여유가 없으며, 오른쪽 형제와 합쳐야 하는 경우
    {
        bptree_merge(parent, pos, pos + 1);
    }
    else if(pos > 0) // 양쪽 모두 여유가 없으며, 왼쪽 형제와 합쳐야 하는 경우
    {
        bptree_merge(parent, pos - 1, pos);
    }
    else 
    {
        printf("Error: No siblings to borrow from or merge with.\n");
    }


}



int bptree_remove(Node** root, int key)
{
    if(root == NULL)
    {
        printf("Error: root is NULL.\n");
        return -1;
    }

    if(*root == NULL)
    {
        printf("Error: root is NULL.\n");
        return -1;
    }

    int result = bptree_remove_helper(root, key);

    // 루트 축소: 키가 0개가 되면 (내부 노드면 유일한 자식, 리프면 NULL)이 새 루트
    if((*root)->keys_count == 0)
    {
        Node* old_root = *root;
        *root = old_root->is_leaf ? NULL : old_root->children[0];

        if (*root)
        {
            (*root)->parent = NULL;
        }
        
        free(old_root);
    }

    return result;
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

