#include "shell/CommandAPI.h"
#include <memory>
#include <curl/curl.h>

namespace shell {

class CurlCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string&,
                std::ostream& out,
                std::ostream& err,
                SysApi&) override
    {
        if (!requireArgs(args, 1, err, 1)) return 1;
        const std::string& url = args[0];

        CURL* curl = curl_easy_init();
        if (!curl) {
            err << "curl: failed to initialize\n";
            return 1;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            err << "curl: " << curl_easy_strerror(res) << "\n";
            curl_easy_cleanup(curl);
            return 1;
        }

        curl_easy_cleanup(curl);
        return 0;
    }

    const char* getName() const override { return "curl"; }
    const char* getDescription() const override { return "HTTP GET request"; }
    const char* getUsage() const override { return "curl <url>"; }

private:
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        std::ostream* out = static_cast<std::ostream*>(userp);
        size_t total = size * nmemb;
        out->write(static_cast<char*>(contents), total);
        return total;
    }
};

std::unique_ptr<ICommand> createCurlCommand() {
    return std::make_unique<CurlCommand>();
}

} // namespace shell