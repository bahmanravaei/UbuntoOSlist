
#include <iostream>

int main(int argc, char* argv[]) {
    if(argc==1){
        std::cout << "\t -list: list of all currently supported Ubuntu releases. \n\t -lts: the current Ubuntu LTS version \n\t -sha256: the sha256 of the disk1.img item of a given Ubuntu release. " << std::endl;

    }else{
        std::cout<<"nothing";
    }
    return 0;
}