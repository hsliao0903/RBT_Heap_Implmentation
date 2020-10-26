/*
 * NAME: Hsiang-Yuan Liao
 * UFID: 4353-5341
 * EMAIL: hs.liao@ufl.edu
 * ADS Project: risingCity
 */

#include <iostream>
#include <cstdio>
#include <string.h>

#define MAX_PERIOD 5 //5 days for a construction period
#define DEBUG 0 //DEBUG use
//#define COUT 1  //print the outcome to standard output

using namespace std;

enum {INSERTCMD = 1, PRINTCMD1, PRINTCMD2}; //three kinds of input commands
enum {BLACK = 1, RED}; //color of the RBTree

//Building Data Node which contains all the parameters we need
struct Node {
    int exec_t;
    int bNum;
    int total_t;
};

//RBTree Node
class RBTNode {
public:
    Node *ptr;     //Point to the data Node
    int color;
    RBTNode *left;
    RBTNode *right;
    RBTNode *parent;

    //Constructor
    RBTNode(void){
        parent = left = right = NULL;
        ptr = NULL;
        // Node is created during insertion
        // Node is red at insertion
        color = RED;
    }

    // check the node is the left child of its parent
    bool isLeftChild(void){
        return this == parent->left;
    }

    //get sibling
    RBTNode *sibling(void){
        // sibling null if no parent
        if(parent == NULL)
            return NULL;

        //it is a left child of its parent, so it's sibling is the right child of its parent
        if(isLeftChild())
            return parent->right;

        return parent->left;
    }

    //get parent's sibling
    RBTNode *PP_sibling(void){
        //must have parent and grandparent
        if (parent == NULL || parent->parent == NULL)
            return NULL;

        //parent is grandparent's left child
        if (parent->isLeftChild())
            return parent->parent->right;

        return parent->parent->left;
    }

    bool hasRedChild(void){
        return (left != NULL && left->color == RED) ||
               (right != NULL && right->color == RED);
    }

};

class RBTree {
    RBTNode *root;

    //left rotation (RR)
    void leftRotate(RBTNode *current){
        //new parent will be current node's right child
        RBTNode *newParent = current->right;

        //check if the current node is the root
        if (current == root)
            root = newParent;

        //let new parent's parent be current node's parent
        newParent->parent = current->parent;
        //set current node's parent's left/right child to new parent
        if(current->parent != NULL){
            if(current->isLeftChild())
                current->parent->left = newParent;
            else
                current->parent->right = newParent;
        }
        //reset current node's parent to new parent
        current->parent = newParent;

        //current node's right child will be new parent's left child
        current->right = newParent->left;
        if (newParent->left != NULL)
            newParent->left->parent = current;

        //new parent's left child will be the current node
        newParent->left = current;
    }

    //right rotation (LL)
    //it is the similar to the left rotation function
    void rightRotate(RBTNode *current){
        RBTNode *newParent = current->left;

        if (current == root)
            root = newParent;

        newParent->parent = current->parent;
        if(current->parent != NULL){
            if(current->isLeftChild())
                current->parent->left = newParent;
            else
                current->parent->right = newParent;
        }

        current->parent = newParent;
        current->left = newParent->right;
        if (newParent->right != NULL)
            newParent->right->parent = current;

        newParent->right = current;
    }

    void colorSwap(RBTNode *x, RBTNode *y){
        int temp_color;
        temp_color = x->color;
        x->color = y->color;
        y->color = temp_color;
    }

    void nodeSwap(RBTNode *x, RBTNode *y){
        Node *temp;
        temp = x->ptr;
        x->ptr = y->ptr;
        y->ptr = temp;
    }

    //fix the tree after insert
    void insert_fix(RBTNode *current){
        //if the current node is root node, then color it to black, and return
        if(current == root){
            current->color = BLACK;
            return;
        }

        //find out the parent, grandparent, and parent's sibling
        RBTNode *pp = current->parent,
                *gp = current->parent->parent,
                *pps = current->PP_sibling();

        //if parent is a black node, then do nothing
        if (pp->color != BLACK){
            if(pps != NULL && pps->color == RED) {
                //if parent's sibling is RED, then change the color, and set new current node to grandparent
                pp->color = BLACK;
                gp->color = RED;
                pps->color = BLACK;
                insert_fix(gp);
            } else {
                //if parent's sibling is BLACK, then there are four cases:
                //do LRb, LLb, RLb, RRb
                if(pp->isLeftChild()){
                    if(current->isLeftChild()){
                        //for LLb
                        rightRotate(gp);
                        colorSwap(pp, gp);
                    } else {
                        //for LRb
                        leftRotate(pp);
                        rightRotate(gp);
                        colorSwap(current, gp);
                    }
                } else {
                    if (current->isLeftChild()) {
                        //RLb
                        rightRotate(pp);
                        leftRotate(gp);
                        colorSwap(current, gp);
                    } else {
                        //RR
                        leftRotate(gp);
                        colorSwap(pp, gp);
                    }
                }
            }
        }//end of if
    }

    //find the rightmost node in the given subtree (which has the biggest reference)
    RBTNode *rightmostnode(RBTNode *x){
        RBTNode *temp = x;
        while(temp->right != NULL)
            temp = temp->right;
        return temp;
    }

    //get the replaced node for delNode
    RBTNode *BSTdelete(RBTNode *x){
        //if it has two children, replace it with the rightmost node in it's left subtree
        if(x->left != NULL && x->right != NULL)
            return rightmostnode(x->left);

        //if it has no child, no need for replacement, return NULL
        if(x->left == NULL && x->right == NULL)
            return NULL;

        //if it has only one child, return that child
        if(x->left != NULL)
            return x->left;
        else
            return x->right;
    }

    //delete the given node in the tree
    void deleteNode(RBTNode *delNode){
        //first do BST delete to get the replaced node y
        RBTNode *y = BSTdelete(delNode);
        RBTNode *py = delNode->parent;  //parent of the delNode

        //delNode is a leaf
        if(y == NULL){
            if(delNode == root){
                root = NULL;
            } else {
                //if delNode is a red node, no rebalancing needed
                if(delNode->color == BLACK){
                    //fix the RBTree
                    rebalanceRBT(delNode);
                }

                //delete the delNode
                if(delNode->isLeftChild()){
                    py->left = NULL;
                } else {
                    py->right = NULL;
                }
            }
            delete delNode;
            return;
        }

        //delNode has only one child
        if(delNode->left == NULL || delNode->right == NULL){
            //if delNode is root
            if(delNode == root){
                //assign y's content to root and delete y
                delNode->ptr = y->ptr;
                delNode->left = delNode->right = NULL;
                delete y;
            } else {
                //delete delNode and replace it by y
                y->parent = py;
                if(delNode->isLeftChild())
                    py->left = y;
                else
                    py->right = y;

                delete delNode;

                if (delNode->color == BLACK && y->color == BLACK){
                    //fix the RBTree
                    rebalanceRBT(y);
                } else {
                    //just let y be BLACK
                    y->color = BLACK;
                }
            }
            return;
        }

        //delNode has two children, swap delNode and y, and do delete y
        nodeSwap(y, delNode);
        deleteNode(y);
    }

    void rebalanceRBT(RBTNode *x){
        if (x == root)
            return;

        RBTNode *sibling = x->sibling(),
                *parent = x->parent;

        if(sibling == NULL){
            //the case that x is a black leaf, move up to py
            rebalanceRBT(parent);
        } else {
            if(sibling->color == RED){
                //sibling is red
                parent->color = RED;
                sibling->color = BLACK;
                if(sibling->isLeftChild()) {
                    rightRotate(parent);
                } else {
                    leftRotate(parent);
                }
                rebalanceRBT(x);
            } else {
                //if sibling is black
                if(sibling->hasRedChild()){
                    //at least one red child
                    if(sibling->left != NULL && sibling->left->color == RED){
                        //might be a red left child or both
                        if(sibling->isLeftChild()){
                            //LL
                            sibling->left->color = sibling->color;
                            sibling->color = parent->color;
                            rightRotate(parent);
                        } else {
                            //RL
                            sibling->left->color = parent->color;
                            rightRotate(sibling);
                            leftRotate(parent);
                        }
                    } else {
                        if(sibling->isLeftChild()){
                            //RL
                            sibling->right->color = parent->color;
                            leftRotate(sibling);
                            rightRotate(parent);
                        } else {
                            //RR
                            sibling->right->color = sibling->color;
                            sibling->color = parent->color;
                            leftRotate(parent);
                        }
                    }
                    parent->color = BLACK;
                } else {
                    //two black children for sibling
                    sibling->color = RED;
                    if(parent->color == BLACK)
                        //Rb0,LB0, case 1, py is BLACK
                        rebalanceRBT(parent);
                    else
                        //Rb0,LB0, case 2, py is RED
                        parent->color = BLACK;
                }
            }
        }
    }

public:
    //constructor
    RBTree(void){
        root = NULL;
    }

    //search for the node by reference (building number)
    //if it is found, then return the node, if not found, return the last node while searching
    RBTNode *search(int bn){
        RBTNode *temp = root;
        //if the tree is empty, return root(NULL)
        //if(temp == NULL)
        //   return root;

        while(temp != NULL){
            if(bn < temp->ptr->bNum) {
                if(temp->left == NULL)
                    return temp;
                else
                    temp = temp->left;
            } else if(bn == temp->ptr->bNum){
                //bNum is unique
                return temp;
            } else {
                if(temp->right == NULL)
                    return temp;
                else
                    temp = temp->right;
            }
        }

        return temp;
    }

    //if it is found, return the data node we are looking for, if it is not found, return NULL
    Node *printSearch(int bn){
        RBTNode *temp = root;
        while(temp != NULL){
            if(bn < temp->ptr->bNum){
                if (temp->left == NULL){
                    //can't find the bNum we want, return NULL
                    return NULL;
                } else
                    temp = temp->left;
            } else if(bn == temp->ptr->bNum){
                //bNum is unique
                break;
            } else {
                if(temp->right == NULL){
                    //can't find the bNum we want, return NULL
                    return NULL;
                } else
                    temp = temp->right;
            }
        }
        if(temp == NULL)
            return NULL;
        else
            return temp->ptr;
    }

    //if it is found, return the first RBTree node, that has the bNum which is bn1 <= bNum <= bn2, if it is not found, return NULL
    RBTNode *inorderRootSearch(int bn1, int bn2){
        RBTNode *temp = root;
        while(temp != NULL){
            if(bn2 < temp->ptr->bNum){
                if (temp->left == NULL){
                    //can't find the bNum we want, return NULL
                    return NULL;
                } else
                    temp = temp->left;
            } else if(bn1 <= temp->ptr->bNum && bn2 >= temp->ptr->bNum){
                //bNum is unique
                break;
            } else {
                if(temp->right == NULL){
                    //can't find the bNum we want, return NULL
                    return NULL;
                } else
                    temp = temp->right;
            }
        }
        if(temp == NULL)
            return NULL;
        else
            return temp;
    }

    //inorder print all the nodes which bNum is between bn1 and bn2
    void inorderPrint(RBTNode *root, int bn1, int bn2, FILE *fout, int *print_comma){
        if(root == NULL)
            return;

        inorderPrint(root->left,bn1, bn2, fout, print_comma);
        if(bn1 <= root->ptr->bNum && bn2 >= root->ptr->bNum){
            if(*print_comma == 1){
                fprintf(fout, ",");
            }
            fprintf(fout, "(%d,%d,%d)", root->ptr->bNum, root->ptr->exec_t, root->ptr->total_t);
            *print_comma = 1;
        }
        inorderPrint(root->right, bn1, bn2, fout, print_comma);

    }
    //insert new data node in RBTree
    void rbtinsert(Node *N){
        RBTNode *newNode = new RBTNode();
        //newNode pointer point to the new data node
        newNode->ptr = N;

        if(root == NULL){
            //if the RBTree is empty
            newNode->color = BLACK;
            root = newNode;
        } else {
            //search where the new node should be inserted, reference: building Number
            RBTNode *temp = search(N->bNum);

            //do nothing if there is a duplicate bNum, well, in this project, it won't be the case
            if (temp->ptr->bNum == N->bNum){
                return;
            }

            //insert it as a child
            newNode->parent = temp;
            //determine if newNode is a left child or a right child
            if(N->bNum < temp->ptr->bNum)
                temp->left = newNode;
            else
                temp->right = newNode;

            //Now, we need to fix the RBTree, if the newNode's parent is a red node
            //possible fixes: XYr and XYb
            insert_fix(newNode);
        }
    }

    //delete the node in RBTree
    void rbtdelete(int bn){
        if (root == NULL)
            return;

        //first make sure bn can be found in the RBTree
        RBTNode *target = search(bn);
        if(target->ptr->bNum != bn)
            return;

        deleteNode(target);
    }
};

//Min Heap Node contains a single pointer, which will point to our building node
struct MinHeapNode {
    Node *ptr;
};

//Min Heap Class
class MinHeap {
    MinHeapNode *heap;
    int volume; //for the heap array size
    int heap_size; //the size of the min heap

public:

    //Constructor for the MinHeap
    MinHeap(int n){
        heap_size = 0;
        volume = n;
        heap = new MinHeapNode[n];
    }

    //swap the MinHeapNode pointer to the data node
    //For example,at first => x point to Node A, y point to Node B, after swap => x point to Node B, y point to Node A
    void swap(MinHeapNode *x, MinHeapNode *y){
        Node *temp;
        temp = x->ptr;
        x->ptr = y->ptr;
        y->ptr = temp;
    }

    //get the index of parent/left child/right child
    //this heap is done by an array
    int parent(int i){
        return (i-1)/2;
    }
    int left(int i){
        return 2*i + 1;
    }
    int right(int i){
        return 2*i + 2;
    }

    //Let the min heap node ptr point to the new data node for heap insertion
    void hinsert(Node *newNode){
        if (heap_size == volume){
            cout << "\nError: Exceed the heap volume\n";
            return;
        }

        if(newNode == NULL)
            return;

        //insert the new min heap node at the end of the heap first
        //the end of the heap is heap_size - 1 cause it is done by an array which the root will be heap[0]
        heap_size++;
        int i = heap_size - 1;
        heap[i].ptr = newNode;

        //fix the min heap to the root, data node's "exec_time" is the min heap reference
        while (i != 0){
            //if it is not the root, then continue to check
            if(heap[parent(i)].ptr->exec_t > heap[i].ptr->exec_t) {
                //parent is bigger, swap with parent
                swap(&heap[i], &heap[parent(i)]);
                //continue to check
                i = parent(i);
            }else if(heap[parent(i)].ptr->exec_t == heap[i].ptr->exec_t){
                //if it is a tie, break the tie by comparing building number
                if(heap[parent(i)].ptr->bNum > heap[i].ptr->bNum){
                    swap(&heap[i], &heap[parent(i)]);;
                    i = parent(i);
                } else
                    break;
            }else
                break;
        }// end of while
    }

    //remove the root from min heap and get the data node that root pointer is pointing at
    Node *removeMin(void){
        if(heap_size <= 0)
            return NULL;

        //only root in the heap, no need to replace root and heapify
        if(heap_size == 1){
            heap_size--;
            return heap[0].ptr;
        }


        Node *root = heap[0].ptr;
        //replace the root by the last element in the heap
        heap[0] = heap[heap_size-1];
        heap_size--;
        //do heapify from root
        heapify(0);

        return root;
    }

    //heapify function, the reference is the exec_time that the heap node pointer is pointing at
    void heapify(int i){
        int l = left(i);
        int r = right(i);
        int smallest = i;  //find the smallest node's index
        int last = heap_size - 1; //the last element's index in the current heap


        if(l <= last){
            //left child is smaller
            if(heap[l].ptr->exec_t < heap[i].ptr->exec_t)
                smallest = l;
                //left child is equal
            else if(heap[l].ptr->exec_t == heap[i].ptr->exec_t){
                if(heap[l].ptr->bNum < heap[i].ptr->bNum)
                    smallest = l;
            }
        }

        if(r <= last){
            //right child is smaller
            if(heap[r].ptr->exec_t < heap[smallest].ptr->exec_t)
                smallest = r;
                //right child is equal
            else if(heap[r].ptr->exec_t == heap[smallest].ptr->exec_t){
                if(heap[r].ptr->bNum < heap[smallest].ptr->bNum)
                    smallest = r;
            }
        }

        //if the smallest is input value i, no need to swap
        //otherwise swap with the smallest node, and do heapify at the smallest node after swap
        if(smallest != i){
            swap(&heap[i], &heap[smallest]);
            heapify(smallest);
        }

    }

    //get the root node ptr
    Node* getMin(void){
        if(heap_size <= 0)
            return NULL;
        else
            return heap[0].ptr;
    }
}; // End of MinHeap class

//Read Input Line by Line and return the Input command type
int readInput(FILE * fp, int *curInputID, int *parameter1, int *parameter2) {

    char InputLineBuf[256];
    char cmd;
    int ret;
    char *pch;

    if(fgets(InputLineBuf, 256, fp) != NULL){
        pch = strtok(InputLineBuf, "()");
        sscanf(pch, "%d: %c", curInputID, &cmd);
        pch = strtok(NULL, "()");
        ret = sscanf(pch, "%d,%d", parameter1, parameter2);

        if(cmd == 'I'){
            //Insert(a,b)
            return INSERTCMD;
        } else if(cmd == 'P' && ret == 1){
            //Printbuilding(a), only get one integer
            return PRINTCMD1;
        } else {
            //Printbuilding(a,b)
            return PRINTCMD2;
        }
    } else {
        //cout << "End of file\n";
        return -1;
    }
}

//Store bNum, exec_time, total_time to a specific Node, which Heap and RBT could access later
Node *CreateNewNode(int para1, int para2){
    Node *N = new Node;
    N->exec_t = 0;  //always 0 for first insertion
    N->total_t = para2;
    N->bNum = para1;
    return N;
}

//execute command function, handle three kind of commands
void ExecuteCommand(int InputType, int parameter1, int parameter2, FILE *fout, MinHeap *h, RBTree *RBT){
    //Execute the input command on the specific day
    //Three command types: INSERTCMD for Insert(a,b), PRINTCMD1 for Printbuilding(a), PRINTCMD2 for Printbuilding(a,b)
    Node *newNode = NULL;
    Node *target_print = NULL;
    if(InputType == INSERTCMD){
        //Insert command
        newNode = CreateNewNode(parameter1, parameter2);
        h->hinsert(newNode);
        RBT->rbtinsert(newNode);
    } else if(InputType == PRINTCMD1){
        //Printbuilding (a) command
        target_print = RBT->printSearch(parameter1);
        //Check if the building exists
        if(target_print != NULL){
            //Print in format (building number,execution time,totalbuildingtime) (a,b,c)
            fprintf(fout, "(%d,%d,%d)\n", target_print->bNum, target_print->exec_t,target_print->total_t);

        } else {
            //the building does not exist
            fprintf(fout, "(0,0,0)\n");
        }
    } else {
        //Printbuilding (a,b) command
        int print_comma = 0; // if there are more than one building found, we need to print a comma first
        RBT->inorderPrint(RBT->inorderRootSearch(parameter1, parameter2), parameter1, parameter2, fout, &print_comma);
        if(print_comma){
            //This means we print at least one building we are looking for, therefore we need to print a new line
            fprintf(fout, "\n");

        } else {
            //no buildings has found in that range
            fprintf(fout, "(0,0,0)\n");
        }

    }

}

//Main function for rising city
int main(int argc, char** argv){
    int curInputID, parameter1, parameter2, EndofFile = 0;
    int InputType = 0, Day = 0, cperiod = 0;
    //Node *newNode = NULL;
    Node *target_node = NULL;
    //Node *target_print = NULL;


    //Initialize MinHap and RBTree
    MinHeap h(2010);
    RBTree RBT;


    //Deal with File I/O, and get fin/fout
    FILE *fin, *fout;
    string inputFile = "";
    string outputFile = "output_file.txt";
    if(argc == 2)
        inputFile = argv[1];
    else{
        cout << "Usage: ./cppfile InputFile\n";
        return 0;
    }

    fin = fopen(inputFile.c_str(), "r");
    if(fin == NULL) {
        cout << "Usage: ./cppfile InputFile\n";
        return 0;
    }
    fout = fopen(outputFile.c_str(), "w");


    // get the first input, the  input format must be "Integer: String(Integer,Integer)" or "Integer: String(Integer)"
    //String could be: Insert or Print or Printbuilding
    InputType = readInput(fin, &curInputID, &parameter1, &parameter2);
    if(InputType <= 0)
        EndofFile = 1;
    //Set flag for getting another Input Line
    //ready_for_new_line = 1;


    //Main Function Loop
    //Terminated until there is no any Node in the heap, and there is no any building is under construction,
    //and we already finish read all the inputs in input file
    for( ; (h.getMin() != NULL) || (target_node != NULL) || !EndofFile ; Day++){
#if DEBUG
        if(Day >= 3346){
            if(target_node != NULL)
                cout << "target_node= " << target_node->bNum << " ,exec_t= " << target_node->exec_t << " ,cperiod= "
                     << cperiod << "\n";
            else
                cout << "target_node is NULL\n";
        }
#endif
        //handle the situation that have multiple commands on the same day, always do the command first
        while(curInputID == Day && !EndofFile){
            ExecuteCommand(InputType, parameter1, parameter2, fout, &h, &RBT);
            InputType = readInput(fin, &curInputID, &parameter1, &parameter2);
            if(InputType <= 0)
                EndofFile = 1;
        }

        //Decide the target building to construct which has the smallest exec_time and building number
        if(target_node == NULL){
            //If there's no node in the heap, then target_node will be NULL
            //If there is, move it out from the heap for construction
            target_node = h.removeMin();

        } else if(target_node->exec_t == target_node->total_t){
            //If the exec time equals total time, then print it out and remove it from the RBT
            //Format would be (building number,Day it is complete) (a,b)
            fprintf(fout, "(%d,%d)\n", target_node->bNum, Day);
            RBT.rbtdelete(target_node->bNum);
            //delete that data node after print it out
            delete target_node;
            //Reset construction period and find the next building for construction immediately
            cperiod = 0;
            target_node = h.removeMin();
        } else if(cperiod >= MAX_PERIOD){
            //If the construction period comes to the MAX_PERIOD=5, then insert that building back into the heap
            h.hinsert(target_node);
            cperiod = 0;
            target_node = h.removeMin();
#if DEBUG
            cout << "Day= "<< Day << "target: bnum=" << target_node->bNum << "exec_t=" << target_node->exec_t << "\n";
#endif

        }

        //For construction process
        if(target_node != NULL){
            target_node->exec_t++;
            cperiod++;
        }

    } //end of while

    fclose(fin);
    fclose(fout);
    return 0;
} //End of Main Function