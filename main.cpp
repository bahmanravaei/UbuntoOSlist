
#include <iostream>
#include <curl/curl.h>
#include <vector>

// A class to record the neccessary information of a product
class UbuntuOs {
public:
    // Constructor
    UbuntuOs(const std::string&p_name, const std::string& a, bool s, const std::string& r, const std::string& d, const std::string& p)
        : product_name(p_name), arch(a), supported(s), release_title(r), disk1_img_sha256(d), pubname(p) {}
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
    virtual std::string getSupportedReleases() const = 0;

    // Return the current Ubuntu LTS version.    
    virtual std::string getCurrentLTSVersion() const = 0;

    // Return the sha256 of the disk1.img item of a given Ubuntu release.
    virtual std::string getDisk1ImgSHA256(const std::string& release) const = 0; 
private:
    std::vector<UbuntuOs> ubuntuOsList;

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
  
    std::string readBuffer="OPS";

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


int main(int argc, char* argv[]) {
    if(argc==1){
        std::cout << "\t -list: list of all currently supported Ubuntu releases. \n\t -lts: the current Ubuntu LTS version \n\t -sha256: the sha256 of the disk1.img item of a given Ubuntu release. " << std::endl;

    }else{
        std::string json_string=fetch_url_data();
        std::cout<<json_string;
    }
    return 0;
}