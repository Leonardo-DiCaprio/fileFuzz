#include<iostream>
#include<vector>
#include<fstream>
#include <iomanip>
#include <algorithm>
using namespace std;


class result_item
{
    public:
        int u;
        int v;
        vector<int> a;
    result_item(int u_,int v_,vector<int> a_)
    {
        u = u_;
        v = v_;
        a = a_;
    }
    bool operator==(const result_item &t1)const{
        if(u!=t1.u) return false;
        if(v!=t1.v) return false;
        for (int k = 0; k < a.size(); k++)
        {
            if(a[k]!=t1.a[k]) return false;
        }
        return true;
    }  
};

class Result
{
    public:
        vector<result_item*> ri;
    bool add(result_item* r)
    {
        for(int k=0;k<ri.size();k++)
        {
            if(*ri[k]==*r) return false;
        }
        ri.push_back(r);
        return true;
    }
    bool add(int u_,int v_,vector<int> a_)
    {
        result_item *tmp = new result_item(u_,v_,a_);
        for(int k=0;k<ri.size();k++)
        {
            if(*ri[k]==*tmp) return false;
        }
        ri.push_back(tmp);
        return true;
    }
};

//vector<int> sbox = { 1,13,7,2,15,10,8,4,0,11,12,14,6,5,3,9 };   //TANGRAM的S盒
//vector<int> sbox = { 5,6,12,10,1,14,7,9,11,0,3,13,8,15,4,2 };   //RECTANGLE的S盒
vector<int> sbox = { 12,10,13,3,14,11,15,7,8,9,1,5,0,2,4,6};   //Midori64的S盒



bool f(int u, int v, vector<int> a)
{
    bool flag = false;
    vector<int>::iterator it;
    int i, n = 0;
    vector<int>  a0, a1;
    for (i = 0; i < a.size(); i++)
    {
        a0.push_back(sbox[a[i] ^ u]);
        a1.push_back(a[i] ^ v);
    }
    for (i = 0; i < a0.size(); i++)
    {
        it = find(a1.begin(), a1.end(), a0[i]);
        if (it != a1.end())
        {
            n++;
        }
    }
    if (n == 4)
    {
        flag = true;
    }



    return flag;
}

bool compare(int a,int b)
{

    return a<b;
}

int main() {

    int i, j, k, iu, iv;
    int count = 0;
    vector<int> a;
    vector<int>result;
    Result *r = new Result;
    ofstream re("result.txt");

    bool f0;


       for (i = 1; i < 15; i++)
       {

        for (j = i + 1; j < 16; j++)
        {
            a.push_back(0);
            a.push_back(i);
            a.push_back(j);
            a.push_back(i^j);
            sort(a.begin (),a.end (),compare);

//            for (k = 0; k < a.size(); k++)
//            {
//
//                cout << a[k] << " ";
//            }
//            cout << endl; //输出A中元素,所有可能情况的A，接下来再进行u和v的遍历筛选

            for (iu = 0; iu < 16; iu++)
            {
                for (iv = 0; iv < 16; iv++)
                {

                    f0 = f(iu, iv, a);
                    if (f0)
                    {

                        if(r->add(iu,iv,a))
                        {
                            count = count+1;
                        }
//                         re <<"u="<< setw(2)<<iu << ", ";
//                         re <<"v="<< setw(2)<<iv << ", ";
//                         re<< "A={";


// //                        re << iu << " ";
// //                        re << iv << " ";

//                         for (k = 0; k < a.size(); k++)
//                         {

//                             re <<a[k] << " ";

//                         }
//                         re<<"}";

//                         re << endl;

                    }


                }
            }

            //删除重复数据并将结果放入result.txt



            a.clear();


        }

    }
    vector<result_item*> unique = r->ri;
    for(vector<result_item*>::iterator it=unique.begin();it!=unique.end();it++)
    {
        re <<"u="<< setw(2)<<(*it)->u << ", ";
        re <<"v="<< setw(2)<<(*it)->v<< ", ";
        re<< "A={";


//                        re << iu << " ";
//                        re << iv << " ";

        for (k = 0; k < (*it)->a.size(); k++)
        {
            re <<(*it)->a[k] << " ";
        }
        re<<"}";
        re << endl;
    }
    cout<<"The number of affine invariant subspace for Sbox:"<<count<<endl;
    return 0;

}
