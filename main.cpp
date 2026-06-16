#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<sstream>
#include<cmath>
using namespace std;

enum class NodeType{//enum means A type with a fixed set of named constant values
    PROJECT,
    SELECT,
    JOIN,
    TABLE
};

class RAnode{
    public:
    NodeType type;
    string value;
    RAnode* left;
    RAnode* right;
    RAnode(NodeType t, string v) : type(t), value(v), right(nullptr), left(nullptr) {}
};

//switch function to convert enum type to string for prnting
string enumTostring(NodeType type){
    switch(type){
        case(NodeType::PROJECT) : return "PROJECT";
        case(NodeType::SELECT) :  return "SELECT";
        case(NodeType::TABLE) : return "TABLE";
        case(NodeType::JOIN) : return "JOIN";
    }
    return "";
}

//print tree
void printtree(RAnode* node,int depth){
    if(node==nullptr)
        return;
    for(int i=0;i<depth;i++)
        cout<< "  ";
    cout<<enumTostring(node->type)<<"("<<node->value<<")"<<endl;
    printtree(node->left,depth+1);
    printtree(node->right,depth+1);
}

string trim(string s){
    int start=0;
    while(start<s.size() && s[start]==' ')
        start++;
    int end=(int)s.size() - 1;
    while(end>=start && s[end]==' ')
        end--;
    return s.substr(start,end-start+1);
    
}

void deletetree(RAnode* node){
    if(node==nullptr)
        return;
    deletetree(node->left);
    deletetree(node->right);
    delete node;
}

RAnode* parser(string& query){
    query=trim(query);
    if(!query.empty() && query.back()==';')
        query.pop_back();
    
    size_t selectpos = query.find("SELECT");
    size_t frompos = query.find("FROM");
    size_t wherepos = query.find("WHERE", frompos);
    size_t joinpos = query.find("JOIN", frompos);
    size_t onpos = query.find("ON", joinpos);
    
    if(selectpos==string::npos || frompos==string::npos){
        cout<<"invalid query"<<endl;
        return 0;
    }
    
    size_t startproj=selectpos+7, starttable=frompos+5;
    string selectstring=trim( query.substr(startproj, frompos-startproj) );//substr gives the sub string from start upto chars
    //substr(10,3) for string rocketracing where r in rocket is at 10 will return roc
    RAnode* projectNode = new RAnode(NodeType::PROJECT, selectstring);
    
    
    if(joinpos==string::npos){//if join is not there
        if(wherepos==string::npos){
            string fromstring=trim(query.substr(starttable));
            projectNode->left= new RAnode(NodeType::TABLE, fromstring);
            }
        
        else{
            size_t startsel=wherepos+6;

            string fromstring=trim(query.substr(starttable,wherepos-starttable));
            string wherestring=trim(query.substr(startsel));

            projectNode->left=new RAnode(NodeType::SELECT,wherestring);
            projectNode->left->left= new RAnode(NodeType::TABLE, fromstring);
        }
    }
    
    
    else{//when join is there
        if(onpos==string::npos){
            cout<<"invalid query"<<endl;
            return 0;
        }
        
        size_t startjoin=joinpos+5, starton=onpos+3;
        
        string fromstring=trim( query.substr(starttable, joinpos-starttable) );
        string secondtablestring=trim( query.substr(startjoin, onpos-startjoin) );
        
        if(wherepos==string::npos){     //if no where
            string onstring=trim(query.substr(starton));
            projectNode->left=new RAnode(NodeType::JOIN, onstring);
            projectNode->left->left= new RAnode(NodeType::TABLE, fromstring);
            projectNode->left->right=new RAnode(NodeType::TABLE, secondtablestring);
        }
        
        else{  //if there is where
            size_t startsel=wherepos+6;
            
            string onstring=trim( query.substr(starton, wherepos-starton) );
            string wherestring=trim(query.substr(startsel));
            
            projectNode->left=new RAnode(NodeType::SELECT,wherestring);
            projectNode->left->left=new RAnode(NodeType::JOIN, onstring);
            projectNode->left->left->left= new RAnode(NodeType::TABLE, fromstring);
            projectNode->left->left->right=new RAnode(NodeType::TABLE, secondtablestring);
        }
    }
    
    return projectNode;
}



string returnTablename(string tablestring){ // function to find table name
    size_t Tablenamepos=tablestring.find(" ");
    if(Tablenamepos==string::npos)
        return tablestring;
    string tablename= trim(tablestring.substr(Tablenamepos+1));
    return tablename;
}


int checkCondition(string condition, string leftname, string rightname){ // to check if the condition is on one table or both
    bool hasleft=0,hasright=0;
    if(condition.find(leftname+".") != string::npos)
        hasleft=true;
    if(condition.find(rightname+".") != string::npos)
        hasright=true;
    if(hasright && !hasleft)
        return 2;
    else if(!hasright && hasleft)           //1->left
        return 1;                           //2->right
    else if(!hasright && !hasleft)          //3->both
        return 0;                           //0->none
    else
        return 3;
}


bool pushdownpossible(RAnode* root){
    if(root==nullptr || root->type!=NodeType::PROJECT)
        return false;
    if(root->left==nullptr || root->left->type!=NodeType::SELECT)
        return false;
    if(root->left->left==nullptr || root->left->left->type!=NodeType::JOIN)
        return false;
    return true;
}


vector<string> checkand(string selectc){  // breaks select statement into multiple selects based on and
    vector<string> result;
    while(selectc.find("AND") != string::npos){
            size_t andpos = selectc.find("AND");
            string a = trim(selectc.substr(0, andpos));
            result.push_back(a);
            selectc = trim(selectc.substr(andpos + 3));
        }
        
    // push the last remaining condition also
    if(!selectc.empty())
        result.push_back(selectc);
        
    return result;
}


RAnode* optimizer(RAnode* root){
    
    if( !pushdownpossible(root))
        return root;
    RAnode* selectNode=root->left;
    RAnode* joinNode = root->left->left;
    string rightT=root->left->left->right->value;
    string leftT=root->left->left->left->value;
    
    string rightName=returnTablename(rightT);
    string leftName=returnTablename(leftT);
    
    vector<string> conditions = checkand(selectNode->value);

    if(conditions.empty())
        conditions.push_back(selectNode->value);

    vector<string> leftconds, rightconds, upperconds;

    for(string cond : conditions){
        int result = checkCondition(cond, leftName, rightName);

        if(result == 1)
            leftconds.push_back(cond);
        else if(result == 2)
            rightconds.push_back(cond);
        else
            upperconds.push_back(cond);   // both tables or none
        }
    
    // push left conditions
    for(int i = 0; i < leftconds.size(); i++){
        RAnode* newSelect = new RAnode(NodeType::SELECT, leftconds[i]);
        newSelect->left = joinNode->left;
        joinNode->left = newSelect;
    }

    // push right conditions
    for(int i = 0; i < rightconds.size(); i++){
        RAnode* newSelect = new RAnode(NodeType::SELECT, rightconds[i]);
        newSelect->left = joinNode->right;
        joinNode->right = newSelect;
    }

    // if no condition remains above join
    if(upperconds.empty()){
        root->left = joinNode;
        delete selectNode;
    }
    else{
        string combined = upperconds[0];
        for(int i = 1; i < upperconds.size(); i++)
            combined += " AND " + upperconds[i];
        selectNode->value = combined;
        selectNode->left = joinNode;
    }

    return root;
}


class Relation{
public:
    vector<string> column;
    vector<vector<string>> row;
};


void printRelation(const Relation& r){
    for(string col : r.column)
        cout << col << " ";
    cout << endl;

    for(auto rw : r.row){
        for(string val : rw)
            cout << val << " ";
        cout << endl;
    }
}


map<string,Relation> database;


vector<string> splitcomma(string projectstring){ // extracts what columns to project
    vector<string> required;
    while(projectstring.find(',') != string::npos){
            size_t commapos = projectstring.find(',');
            string a = trim(projectstring.substr(0, commapos));
            required.push_back(a);
            projectstring = trim(projectstring.substr(commapos + 1));
        }
        
    // push the last remaining condition also
    if(!projectstring.empty())
        required.push_back(projectstring);
        
    return required;
}


void splitselect(string complete,string& col,string& op, string& val){
    //extract conditional operand, attribute and condition value
    if(complete.find("<=") != string::npos){
        size_t pos=complete.find("<=");
        col= trim(complete.substr(0,pos));
        op="<=";
        val=trim(complete.substr(pos+2));
    }
    else if(complete.find(">=") != string::npos){
        size_t pos=complete.find(">=");
        col= trim(complete.substr(0,pos));
        op=">=";
        val=trim(complete.substr(pos+2));
    }
    else if(complete.find('<') != string::npos){
        size_t pos=complete.find('<');
        col= trim(complete.substr(0,pos));
        op="<";
        val=trim(complete.substr(pos+1));
    }
    else if(complete.find('>') != string::npos){
        size_t pos=complete.find('>');
        col= trim(complete.substr(0,pos));
        op=">";
        val=trim(complete.substr(pos+1));
    }
    else if(complete.find('=') != string::npos){
        size_t pos=complete.find('=');
        col= trim(complete.substr(0,pos));
        op="=";
        val=trim(complete.substr(pos+1));
    }
}


bool isNumber(string s){
    if(s.empty()) return false;
    for(char c : s){
        if(!isdigit(c)) return false;
    }
    return true;
}



void parsejoin(string joincondition, string& col1, string& col2){
    size_t condpos= joincondition.find("=");
    col1= trim(joincondition.substr(0,condpos));
    col2= trim(joincondition.substr(condpos+1));
}


string removeQuotes(string s){
    if(s.size() >= 2 && s.front() == '\'' && s.back() == '\'')
        return s.substr(1, s.size() - 2);
    return s;
}


Relation executor(RAnode* root){
    if(root==nullptr)
        return Relation();
    if(root->type== NodeType::TABLE)
        return database[returnTablename(root->value)];
    
    else if(root->type== NodeType::PROJECT){
        Relation child= executor(root->left);
        Relation result;
        vector<string> required= splitcomma(root->value);
        // required it the array of columns that query want to project
        vector<int> index;
        for(int k=0;k<required.size(); k++){
            for(int i=0; i<child.column.size() ;i++){
                if( child.column[i]==required[k] ){
                    index.push_back(i);
                    result.column.push_back(child.column[i]);
                    break;
                }
            }
        }
        for(const auto& r : child.row){
            vector<string> newrow;
            for(int i=0;i<index.size();i++)
                newrow.push_back(r[index[i]]);
            result.row.push_back(newrow);
        }
        return result;
    }
    
    else if(root->type== NodeType::SELECT){
        Relation child=executor(root->left);
        Relation result;
        result.column=child.column;
        vector<string> conditions = checkand(root->value);
        // conditions contain all the conditions that were seperated by and in the original select condition
        for(const auto& r : child.row){
            bool alltrue = true;

            for(string condition : conditions){
                string a, op, val;
                splitselect(condition,a,op,val);

                int index=-1;
                for(int i=0;i<child.column.size();i++){
                    if(child.column[i]==a){
                        index=i;
                        break;
                    }
                }

                if(index == -1){
                    cout << "Column not found: " << a << endl;
                    alltrue = false;
                    break;
                }

                string left = r[index];
                string right = removeQuotes(val);

                bool conditiontrue = false;

                if(isNumber(left) && isNumber(right)){
                int x = stoi(left);
                int y = stoi(right);

                if(op == "<" && x < y)
                    conditiontrue = true;
                else if(op == ">" && x > y)
                    conditiontrue = true;
                else if(op == "=" && x == y)
                    conditiontrue = true;
                else if(op == "<=" && x <= y)
                    conditiontrue = true;
                else if(op == ">=" && x >= y)
                    conditiontrue = true;
                }
                else{
                    if(op == "=" && left == right)
                        conditiontrue = true;
                }

                if(conditiontrue == false){
                    alltrue = false;
                    break;
                }
            }

            if(alltrue)
                result.row.push_back(r);
        }

        return result;
    }
    
    else if(root->type==NodeType::JOIN){
        Relation child1= executor(root->left);
        Relation child2= executor(root->right);
        Relation result;
        //result.column=child1.column + child2.column;
        string column1, column2;
        parsejoin(root->value, column1, column2);
        
        int idx1=-1,idx2=-1;
        for(int i=0;i<child1.column.size();i++){
            if(child1.column[i]==column1){
                idx1=i;
                break;
            }
        }
        for(int i=0;i<child2.column.size();i++){
            if(child2.column[i]==column2){
                idx2=i;
                break;
            }
        }
        if(idx1 == -1 || idx2 == -1){
            cout << "Join column not found\n";
            return result;
        }
        
        result.column = child1.column;

        for(int i = 0; i < child2.column.size(); i++){
            if(i != idx2)
                result.column.push_back(child2.column[i]);
        }
        
        for(auto& r : child1.row){
            for(auto& r2 : child2.row){
                if(r[idx1]==r2[idx2]){
                    vector<string> newrow=r;
                    for(int i = 0; i < r2.size(); i++){
                        if(i != idx2)
                            newrow.push_back(r2[i]);
                    }
                    result.row.push_back(newrow);
                }
            }
        }
        
        return result;
        
    }
    return Relation();
}


const int pagesize=256, columnsize=8;


class costinfo{
public:
    long estimatedRows =0;
    vector<string> outputColumns;
    int outputPages;
    int totalCost=0;
};


int costFormula(long row,long columns){
    return ceil((double)(row*columns*columnsize)/pagesize);
}


costinfo costCalc(RAnode* root){
    if(root==nullptr)
        return costinfo();
    
    if(root->type== NodeType::TABLE){
        costinfo result;
        Relation r = database[returnTablename(root->value)];
        result.estimatedRows= r.row.size();
        result.outputColumns=r.column;
        result.totalCost=costFormula(result.estimatedRows, result.outputColumns.size());
        return result;
    }
    
    if(root->type== NodeType::PROJECT){
        costinfo childcost=costCalc(root->left);
        costinfo result;
        result.estimatedRows= childcost.estimatedRows;
        result.outputColumns = splitcomma(root->value);
        result.totalCost=costFormula(result.estimatedRows, result.outputColumns.size());
        result.totalCost = result.totalCost + childcost.totalCost;
        return result;
    }
    
    if(root->type== NodeType::SELECT){
        costinfo childcost = costCalc(root->left);
        Relation selected = executor(root);
        costinfo result;
        result.estimatedRows = selected.row.size();
        result.outputColumns = childcost.outputColumns;
        
        result.totalCost=costFormula(result.estimatedRows, result.outputColumns.size());
        result.totalCost = result.totalCost + childcost.totalCost;
        return result;
    }
    
    if(root->type == NodeType::JOIN){
        costinfo leftcost = costCalc(root->left);
        costinfo rightcost = costCalc(root->right);

        Relation joined = executor(root);

        costinfo result;
        result.estimatedRows = joined.row.size();
        result.outputColumns = joined.column;
        result.outputPages = costFormula(result.estimatedRows, result.outputColumns.size());

        int leftOuterCost =
            leftcost.outputPages + leftcost.outputPages * rightcost.outputPages;

        int rightOuterCost =
            rightcost.outputPages + rightcost.outputPages * leftcost.outputPages;

        int joinProcessingCost = min(leftOuterCost, rightOuterCost);

        result.totalCost =
            leftcost.totalCost +
            rightcost.totalCost +
            joinProcessingCost +
            result.outputPages;

        return result;
    }
    return costinfo();
}



vector<string> splitCSVLine(string line){
    vector<string> result;
    string value;
    stringstream ss(line);

    while(getline(ss, value, ',')){
        value = trim(value);

        // remove Windows/Mac carriage return if present
        if(!value.empty() && value.back() == '\r')
            value.pop_back();

        result.push_back(value);
    }

    return result;
}


Relation loadCSV(string filename){
    Relation r;
    ifstream file(filename);

    if(!file.is_open()){
        cout << "Could not open file: " << filename << endl;
        return r;
    }

    string line;

    // first line = column names
    if(getline(file, line)){
        r.column = splitCSVLine(line);
    }

    // remaining lines = rows
    while(getline(file, line)){
        if(line.empty())
            continue;

        vector<string> row = splitCSVLine(line);
        r.row.push_back(row);
    }

    file.close();
    return r;
}


int main(){
    
    database["EMP"] = loadCSV("EMP.csv");
    database["DEPT"] = loadCSV("DEPT.csv");
    database["PROJECT"] = loadCSV("PROJECT.csv");
    database["CLIENT"] = loadCSV("CLIENT.csv");
    database["ASSIGNMENT"] = loadCSV("ASSIGNMENT.csv");
    database["WORKS_ON"] = loadCSV("WORKS_ON.csv");
    
    string query, line;

    while(getline(cin, line)){
        query += " " + line;

        if(line.find(";") != string::npos)
            break;
    }
    
    RAnode* root= parser(query);
    
    cout<<"Original RA tree :"<<endl;
    printtree(root,0);
    
    
    costinfo originalCost = costCalc(root);
    cout << endl << "Original Cost: " << originalCost.totalCost << " pages" << endl;
    
    root=optimizer(root);
    
    cout<<endl<<endl<<"Optimised RA tree :"<<endl;
    printtree(root, 0);
    
    costinfo optimizedCost = costCalc(root);
    cout << endl << "Optimised Cost: " << optimizedCost.totalCost << " pages" << endl;
    
    Relation ans = executor(root);
    cout<<endl<<endl<<"Final Result :"<<endl;
    printRelation(ans);
    cout << endl << "Query execution completed successfully." << endl;
    deletetree(root);
    
    return 0;
}


