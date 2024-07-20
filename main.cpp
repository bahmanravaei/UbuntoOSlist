
#include <iostream>
#include <curl/curl.h>
#include <vector>
#include <nlohmann/json.hpp>
using json = nlohmann::json;


// A class to record the neccessary information of a product
class UbuntuOs {
public:
    // Constructor
    UbuntuOs(const std::string&p_name, const std::string& a, bool s, const std::string& r, const std::string& d, const std::string& p)
        : product_name(p_name), arch(a), supported(s), release_title(r), disk1_img_sha256(d), pubname(p) {}

    std::string get_product_name(){return product_name;} 
    std::string get_arch(){return arch;}
    bool get_supported(){return supported;} 
    std::string get_release_title(){return release_title;} 
    std::string get_disk1_img_sha256(){return disk1_img_sha256;} 
    std::string get_pubname(){return pubname;}       
private:
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
    virtual std::string getCurrentLTSVersion() = 0;

    // Return the sha256 of the disk1.img item of a given Ubuntu release.
    virtual std::string getDisk1ImgSHA256(const std::string& release) = 0; 
//private:
    std::vector<UbuntuOs> ubuntuOsList;

};


//Define the drived class and define the virtual functions
class UbuntuInterfaceImpl : public UbuntuInterface {
public:
    UbuntuInterfaceImpl() {}
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

    std::string getCurrentLTSVersion() override{
        return std::string("Test getCurrentLTSVersion");
    }

    std::string getDisk1ImgSHA256(const std::string& release) override{
        return std::string("Test getDisk1ImgSHA256");
    }
    
    //Function to extract the data from the string and parse it and populate the ubuntuOsList
    std::vector<UbuntuOs> extractUbuntuOsData(std::string& jsonString) {
        
 
        
        // Parse the JSON string
        auto j = json::parse(jsonString);

        //return ubuntuOsList;

        // Iterate over the products
        for (const auto& [key, product] : j["products"].items()) {

            std::string product_name = key;  // Get the product name (key)
            std::string arch = product["arch"];
            std::string release_title = product["release_title"];
            bool support = false;
            if (product.contains("supported")){
                //std::cout<<"supported: "<<product["supported"];
                support = product["supported"];
                //std::cout<< "---" <<support<< std::endl;
            }
            
            std::string disk1_img_sha256="";
            std::string pubname = "";
            // Extract the sha256 of "disk1.img"
            for (auto& version : product["versions"].items()) {
                auto items = version.value()["items"];
                
                if (items.contains("disk1.img")) {
                    disk1_img_sha256 = items["disk1.img"]["sha256"];
                }
                // Extract the pubname
                pubname = version.value()["pubname"];
                
            }
            //std::cout <<"product_name: " << product_name << std::endl ;
            //std::cout << "\t" << arch << ", " << release_title << ", " << disk1_img_sha256 << ", " << std::endl;
            //std::cout<<std::endl <<"\t  pubname:" << pubname << std::endl;

            //continue;

            // Create an instance of UbuntuOs and add it to the list
            UbuntuOs ubuntuOs(product_name, arch, support, release_title, disk1_img_sha256, pubname);
            ubuntuOsList.push_back(ubuntuOs);

            //std::cout<<"+++ \t end of loop\t +++ \n";
        }
        return ubuntuOsList;
    }

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

// Download the json data from the url
std::string fetch_url_data() {
    CURL* curl;
    CURLcode res;
  
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://cloud-images.ubuntu.com/releases/streams/v1/com.ubuntu.cloud:released:download.json");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return "OPS";
        } else {
            //std::cout << "Response Data: " << readBuffer << std::endl;
            return readBuffer;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return readBuffer;
}

void print_list(std::vector<std::string> item_list)
{
    for (const auto& name : item_list) {
            std::cout << name << std::endl;
        }
}


int main(int argc, char* argv[]) {

    if(argc==1){
        std::cout << "\t -list: list of all currently supported Ubuntu releases. \n\t -lts: the current Ubuntu LTS version \n\t -sha256: the sha256 of the disk1.img item of a given Ubuntu release. " << std::endl;

    }else{
        UbuntuInterfaceImpl* Ubuntu_data_list = new UbuntuInterfaceImpl();
        std::vector<UbuntuOs> ubuntuOsList;

        std::string results=fetch_url_data();
        //std::cout<<json_string;
        if (results.compare("OPS")==0)
            std::cout<<"Something is wrong!"<< std::endl;
        else{
            ubuntuOsList = Ubuntu_data_list->extractUbuntuOsData(results);
            //Ubuntu_data_list->printOsList();
            std::string option = argv[1];
            if(option.compare("-list")==0){
                std::cout<<"List of all currently supported Ubuntu releases!"<<std::endl;
                std::vector<std::string> productNames = Ubuntu_data_list->getSupportedReleases();
                print_list(productNames);
            }
        }

    }
    return 0;
}