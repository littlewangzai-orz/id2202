#include <iostream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

//tree's struct
struct biTree{
    biTree* left;
    biTree* right;
    string val;

    biTree(string val): val(val), left(nullptr), right(nullptr){}
};

void CreateBiTree(biTree* &T){
    string data;
    getline(cin,data);
    istringstream ss(data);

    string type, value;

    if(getline(ss,type,':') && getline(ss,value)){
        if( type == "Node"){
            T = new biTree(data);
            CreateBiTree(T->left);
            CreateBiTree(T->right);
        }
        else if (type == "Leaf"){
            T = new biTree(data);
        }
    }
    //else       
}
void preTree(biTree* T){
    if(T == NULL){
        return;
    }
    else {
        cout << T -> val << endl;
        preTree(T->left);
        preTree(T->right);
    }
}
void inTree(biTree* T){
    if(T == NULL){
        return;
    }
    else {
        inTree(T->left);
        cout << T -> val << endl;
        inTree(T->right);
    }
}
void postTree(biTree* T){
    if(T == NULL){
        return;
    }
    else {
        postTree(T->left);
        postTree(T->right);
        cout << T -> val << endl;
    }
}
int sizeofTree(biTree* T){
    int count = 0;
    if(T == NULL){
        return 0;
    }
    else {
        sizeofTree(T->left);
        sizeofTree(T->right);
    }
    return 1 + sizeofTree(T -> left) + sizeofTree( T -> right);
}
void listofTree(biTree* T,vector<int>& leaves){
    //vector<int> list;
    if(T == NULL){
        return;
    }
    else{
        listofTree(T -> left,leaves);
        listofTree(T -> right,leaves);
        if( T -> left == nullptr && T -> left == nullptr){
            //change string to int try to use stoi
            string data = T -> val;
            data.erase(0, data.find(':') + 1); 
            leaves.push_back(stoi(data));
            //list.push_back(stoi(data));
        }
    }
}
int depthofTree(biTree* T){
    if(T == NULL){
        return 0;
    }
    else {
        int left_height = depthofTree(T->left);
        int right_height = depthofTree(T->right);
        int height = ( left_height > right_height) ? left_height : right_height;
        return ++height;
    }
    
}

int main(int argc, char* argv[]) {
    
    
    
    if (argc != 2) {
        //cerr << "Error: Invalid number of arguments." << endl;
        return 1;
    }
    string command = argv[1];
    biTree* root = NULL;
    vector<int> leaves;
    if (command != "pre-order" && command != "in-order" && command != "post-order" 
                && command != "size" && command != "list" && command != "depth"  )   
    {
        //cerr << "wrong command!" << endl;
        return 1;
    }
    
    else {
        CreateBiTree(root);
        //if tree == NULL
        if( root == NULL){
            //cerr << "no tree !" << endl;
            return 1;
        }
        //
        else if( command == "pre-order"){
            //cout << "the output of the pre-order Tree is: " << endl;
            preTree(root);
        }
        else if ( command == "in-order"){
            //cout << "the output of the in-order Tree is: " << endl;
            inTree(root);
        } 
        else if ( command == "post-order"){
            //cout << "the output of the post-order Tree is: " << endl;
            postTree(root);
        }
        else if ( command == "list"){
            //cout << "the list of the Tree is:" << endl;
            listofTree(root,leaves);
            for (vector<int>::iterator it = leaves.begin(); it != leaves.end(); ++it) {
                if(it == leaves.end() -1)
                    cout << *it << endl;
                else 
                    cout << *it << ","; // 使用 * 操作符解引用迭代器，获取元素的值
            }
            
            
        }
        else if ( command == "size"){
            //cout << "the size of the Tree is:" << endl;
            cout << sizeofTree(root) << endl;
        }
            
        else if ( command == "depth"){
                //cout << "the depth of the tree is: " << endl;
                cout << depthofTree(root)-1 << endl;
        }
    }
    return 0;
}