
#include <iostream>
#include <curl/curl.h>
#include <vector>
#include <nlohmann/json.hpp>
#include <cstdlib>
using json = nlohmann::json;


// A class to record the neccessary information of a product
class UbuntuOs {
public:
    // Constructor
    UbuntuOs(const std::string&p_name, const std::string& a, bool s, const std::string& r, const std::string& d, const std::string& p)
        : product_name(p_name), arch(a), supported(s), release_title(r), disk1_img_sha256(d), pubname(p) {}

    // Getter methods for product attributes
    std::string get_product_name(){return product_name;} 
    std::string get_arch(){return arch;}
    bool get_supported(){return supported;} 
    std::string get_release_title(){return release_title;} 
    std::string get_disk1_img_sha256(){return disk1_img_sha256;} 
    std::string get_pubname(){return pubname;}       

private:
    // Product attributes
    //TODO: other variable field can be added for the future application
    std::string product_name;
    std::string arch;
    bool supported;
    std::string release_title;
    std::string disk1_img_sha256;
    std::string pubname;


};


// An interface to define some virtual function (based on project description)
// This part is a pretty ambigous for me! the role of Interface! why it is neccessary!
// I think: you want to test my knowledge about the Interface and virtual function. ;)
class UbuntuInterface {
public: 
    // Virtual destructor for proper cleanup of derived classes
    virtual ~UbuntuInterface() =  default;

    // Return a list of all currently supported Ubuntu releases.
    virtual std::vector<std::string> getSupportedReleases() = 0 ;

    // Return the current Ubuntu LTS version.    
    virtual std::vector<std::string> getCurrentLTSVersion() = 0;

    // Return the sha256 of the disk1.img item of a given Ubuntu release.
    virtual std::string getDisk1ImgSHA256(const std::string& release) = 0; 
//private:
    // List to store Ubuntu OS objects
    std::vector<UbuntuOs> ubuntuOsList;

};


// Define the derived class and implement the virtual functions
class UbuntuInterfaceImpl : public UbuntuInterface {
public:
    // Constructor
    UbuntuInterfaceImpl() {}

    ~UbuntuInterfaceImpl() {
        ubuntuOsList.clear();
        ubuntuOsList.shrink_to_fit();                    
    }
    
    // Implement getSupportedReleases to return names of supported products
    std::vector<std::string> getSupportedReleases() override {
        std::vector<std::string> productNames;
        for (auto& os : ubuntuOsList) {
            //std::cout<< os.get_product_name()<< "\t";
            //bool supported = os.get_supported();
            if(os.get_supported()){
                productNames.push_back(os.get_product_name());
            }
        }
        return productNames;
    }

    // Implement getCurrentLTSVersion to return names of LTS products
    std::vector<std::string> getCurrentLTSVersion() override {
        std::vector<std::string> productNames;
        for (auto& os : ubuntuOsList) {
            //std::cout<< os.product_name<< "\t";
            if(os.get_supported()){
                if(os.get_release_title().find("LTS")){
                    productNames.push_back(os.get_product_name());
                }
            }
        }        
        return productNames;
    }


    // Implement getDisk1ImgSHA256 to return the sha256 of a specific product's disk1.img
    std::string getDisk1ImgSHA256(const std::string& release) override{
        std::string sha256="The wrong product name";
        for (auto& os : ubuntuOsList) {
            if(os.get_product_name().compare(release)==0){
                sha256 = os.get_disk1_img_sha256();
            }
        }
        return sha256;
    }
    
    // Function to extract data from a JSON string and populate the ubuntuOsList
    std::vector<UbuntuOs> extractUbuntuOsData(std::string& jsonString) {

        try{

            // Parse the JSON string
            auto j = json::parse(jsonString);

    
            // Iterate over the products
            for (const auto& [key, product] : j["products"].items()) {

                std::string product_name = key;  // Get the product name (key)
                std::string arch = product["arch"];
                std::string release_title = product["release_title"];
                bool support = false;
                std::string disk1_img_sha256="";
                std::string pubname = "";
        
                if (product.contains("supported")){
                    support = product["supported"];
                }
                
                
                // Extract the sha256 of "disk1.img"
                for (auto& version : product["versions"].items()) {
                    auto items = version.value()["items"];
                    
                    if (items.contains("disk1.img")) {
                        disk1_img_sha256 = items["disk1.img"]["sha256"];
                    }
                    // Extract the pubname
                    pubname = version.value()["pubname"];
                    
                }
                
                // Create an instance of UbuntuOs and add it to the list
                UbuntuOs ubuntuOs(product_name, arch, support, release_title, disk1_img_sha256, pubname);
                ubuntuOsList.push_back(ubuntuOs);

            }
        } catch(...){
            std::cerr << "Json parse error!"<<std::endl;
            exit(EXIT_FAILURE);
        }
        return ubuntuOsList;
    }


    // Function to print the list of all OS products
    void printOsList() {
        std::cout<< "List of all OS products:"<<std::endl;
        for (auto& os : ubuntuOsList) {
            std::cout<< os.get_product_name()<< "\t";
            
        }
        std::cout<<std::endl;
    }

};

// Callback function to write data to a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Download the json data from the URL
std::string fetch_url_data() {
    CURL* curl;
    CURLcode res;
  
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    try{
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, "https://cloud-images.ubuntu.com/releases/streams/v1/com.ubuntu.cloud:released:download.json");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform(curl);

            if(res != CURLE_OK) {
                throw std::runtime_error("CURLE PROBLEM");
            } else {
                //std::cout << "Response Data: " << readBuffer << std::endl;
                return readBuffer;
            }

            curl_easy_cleanup(curl);
        }
    }
    catch (const std::runtime_error& e) {
        // Code to handle the exception
        std::cerr<< "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        exit(EXIT_FAILURE);
    }catch (...) {
        // Code to handle any other exceptions
        std::cerr << "Caught an unknown exception" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    curl_global_cleanup();
    return readBuffer;
}


// Function to print a list of strings -  This funtion is used to print the product_name list
void print_list(std::vector<std::string> item_list)
{
    for (const auto& name : item_list) {
            std::cout << name << std::endl;
        }
}


int main(int argc, char* argv[]) {
    // If there are no command line arguments, display hints for the user.
    if(argc==1){
        std::cout << "\t -list: list of all currently supported Ubuntu releases. \n\t -lts: the current Ubuntu LTS version \n\t -sha256 [product_name]: the sha256 of the disk1.img item of a given Ubuntu release. " << std::endl;

    }
    else{
        // There is a command line argument, and based on this argument, a specific task will be executed.

        // Create an instance of UbuntuInterfaceImpl as core object to call different functions on it.
        UbuntuInterfaceImpl* Ubuntu_data_list = new UbuntuInterfaceImpl();
        std::vector<UbuntuOs> ubuntuOsList;

        // Fetch the json data from the specified URL.
        std::string results=fetch_url_data();
        
        if (results.compare("OPS")==0)
            std::cout<<"Something is wrong!"<< std::endl;
        else{

            // Extract data from the returned json string and populate the Ubuntu_data_list with information about different products.
            ubuntuOsList = Ubuntu_data_list->extractUbuntuOsData(results);
            //Ubuntu_data_list->printOsList();
            std::string option = argv[1];
            
            // Return a list of all currently supported Ubuntu releases.
            if(option.compare("-list")==0){
                std::cout<<"List of all currently supported Ubuntu releases!"<<std::endl;
                std::vector<std::string> productNames = Ubuntu_data_list->getSupportedReleases();
                print_list(productNames);
            }// Return the current Ubuntu LTS version.
            else if(option.compare("-lts")==0){
                std::cout<<"The current LTS version!"<<std::endl;
                std::vector<std::string> productNames = Ubuntu_data_list->getCurrentLTSVersion();
                print_list(productNames);

            } // Return the sha256 of the disk1.img item of a given Ubuntu release.
            else if(option.compare("-sha256")==0){
                if(argc==2){
                    std::cout<<"Specify the name of a product!" <<std::endl;
                }else if(argc==3){
                    std::string a_product = argv[2];
                    std::string sha256 = Ubuntu_data_list->getDisk1ImgSHA256(a_product);
                    std::cout<< "Sha256 of the disk1.img item of a given Ubuntu release:\n";
                    std::cout<< sha256 << std::endl;
                }
            }
            else{
                std::cout<<"Wrong command line argument. please check it again!"<< std::endl;
            }
        }
        delete Ubuntu_data_list;

    }    
    return 0;
}