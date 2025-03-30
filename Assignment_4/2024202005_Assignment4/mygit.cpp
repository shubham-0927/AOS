#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <zlib.h>
#include <fstream>
#include<openssl/sha.h>
#include <iomanip>
#include <sstream>

#include <sys/stat.h>

#include <arpa/inet.h> 

#include <algorithm>
using namespace std;
namespace fs = filesystem;


struct IndexEntry {
    string mode; 
    string sha1; 
    string filename; 
};
struct TreeEntry {
    string mode; 
    string name; 
    string sha1; 
    uint32_t size;
    uint32_t ctime;
    uint32_t mtime;
};

struct CommitLogEntry {
    string parentSHA;
    string currentSHA;
    string authorName;
    string authorEmail;
    string timestamp;
    string commitMessage;
};


string calculate_sha1(const string& data) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), hash);

    stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        ss <<hex<<setw(2)<<setfill('0')<<static_cast<int>(hash[i]);
    }
    return ss.str();
}


void write_object(const string& hash, const string& content) {
    string dir = ".git/objects/" + hash.substr(0, 2);
    string filepath = dir + "/" + hash.substr(2);
    fs::create_directories(dir);

    
    vector<char> compressedData(1024);
    z_stream deflateStream;
    deflateStream.zalloc = Z_NULL;
    deflateStream.zfree = Z_NULL;
    deflateStream.opaque = Z_NULL;

    deflateInit(&deflateStream, Z_BEST_COMPRESSION);
    deflateStream.next_in = (Bytef*)content.data();
    deflateStream.avail_in = content.size();

    string compressedContent;
    do {
        deflateStream.next_out = (Bytef*)compressedData.data();
        deflateStream.avail_out = compressedData.size();
        deflate(&deflateStream, Z_FINISH);
        compressedContent.append(compressedData.data(), compressedData.size() - deflateStream.avail_out);
    } while (deflateStream.avail_out == 0);

    deflateEnd(&deflateStream);

    ofstream outFile(filepath, ios::binary);
    outFile.write(compressedContent.data(), compressedContent.size());
    outFile.close();
}
string compute_sha1(const string& data) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), hash);
    
    stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        ss << hex << setw(2) << setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

void write_blob(const string& content) {
    string sha1 = compute_sha1(content);
    string dir = sha1.substr(0, 2);
    string blob_sha = sha1.substr(2);
    string path = "./.git/objects/" + dir + "/" + blob_sha;

    fs::create_directories("./.git/objects/" + dir);
    

    uLongf compressedSize = compressBound(content.size());
    vector<Bytef> compressedData(compressedSize);
    if (compress(compressedData.data(), &compressedSize, reinterpret_cast<const Bytef*>(content.data()), content.size()) != Z_OK) {
        cerr << "Failed to compress data\n";
        return;
    }
    
    ofstream file(path, ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(compressedData.data()), compressedSize);
        file.close();
    } else {
        cerr << "Failed to create blob file\n";
    }
}
string hex_to_bytes(const string& hex) {
    string bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        string byte_string = hex.substr(i, 2);
        char byte = static_cast<char>(strtol(byte_string.c_str(), nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}
string write_tree(const fs::path& dir) {
    vector<TreeEntry> entries;

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.path().filename() == ".git"||entry.path().filename() == "mygit"|| entry.path().filename() == "mygit.cpp") continue;

        TreeEntry tree_entry;
        if (entry.is_directory()) {
            tree_entry.mode = "40000";
            tree_entry.name = entry.path().filename().string();
            tree_entry.sha1 = write_tree(entry.path());
        } else if (entry.is_regular_file()) {
            tree_entry.mode = "100644";
            tree_entry.name = entry.path().filename().string();
            
            // Read file content into a string
            ifstream file(entry.path(), ios::binary);
            string file_content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            
            tree_entry.sha1 = compute_sha1("blob " + to_string(file_content.size()) + '\0' + file_content);
            write_blob("blob " + to_string(file_content.size()) + '\0' + file_content);
        }
        entries.push_back(tree_entry);
    }

    sort(entries.begin(), entries.end(), [](const TreeEntry& a, const TreeEntry& b) {
        return a.name < b.name;
    });

    stringstream tree_content;
    for (const auto& entry : entries) {
        tree_content << entry.mode << " " << entry.name << '\0' << hex_to_bytes(entry.sha1);
    }
    string content = "tree " + to_string(tree_content.str().size()) + '\0' + tree_content.str();
    string tree_sha1 = compute_sha1(content);
    write_blob(content);

    return tree_sha1;
}

void write_tree_command() {
    string root_tree_sha1 = write_tree(".");
    cout << root_tree_sha1 << endl;
}

string decompress(const string& compressedData) {
    z_stream strm = {};
    strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressedData.data()));
    strm.avail_in = compressedData.size();

    if (inflateInit(&strm) != Z_OK) {
        cerr << "Failed to initialize zlib.\n";
        return "";
    }

    vector<char> decompressedData(4096);
    string output;
    int ret;
    do {
        strm.next_out = reinterpret_cast<Bytef*>(decompressedData.data());
        strm.avail_out = decompressedData.size();

        ret = inflate(&strm, Z_NO_FLUSH);
        if (ret == Z_STREAM_ERROR || ret == Z_MEM_ERROR || ret == Z_DATA_ERROR) {
            cerr << "Decompression error.\n";
            inflateEnd(&strm);
            return "";
        }
        output.append(decompressedData.data(), decompressedData.size() - strm.avail_out);
    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    return output;
}

void ls_tree(const string& tree_sha, bool name_only) {
    string dir = tree_sha.substr(0, 2);
    string blob_sha = tree_sha.substr(2);
    string path = "./.git/objects/" + dir + "/" + blob_sha;

    ifstream file(path, ios::binary);
    if (!file.is_open()) {
        cerr << "Can't open tree object file at path: " << path << endl;
        return;
    }

    vector<char> compressedData((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    string object_str = decompress(string(compressedData.begin(), compressedData.end()));
    if (object_str.empty()) {
        cerr << "Decompressed data is empty.\n";
        return;
    }
    size_t header_end = object_str.find('\0');
    if (header_end == string::npos) {
        cerr << "Malformed tree object header.\n";
        return;
    }
    size_t pos = header_end + 1;
    while (pos < object_str.size()) {
        // Extract mode (e.g., "100644" or "40000")
        size_t space_pos = object_str.find(' ', pos);
        if (space_pos == string::npos) break;
        string mode = object_str.substr(pos, space_pos - pos);
        pos = space_pos + 1;

        size_t null_pos = object_str.find('\0', pos);
        if (null_pos == string::npos) break;
        string name = object_str.substr(pos, null_pos - pos);
        pos = null_pos + 1;

        if (pos + 20 > object_str.size()) break;
        pos += 20;

        if (name_only) {
            cout << name << endl;
        } else {
            string type = (mode == "40000") ? "tree" : "blob";
            cout << mode << " " << type << " " << name << endl;
        }
    }
}

vector<char> compress_data(const string& data) {
    uLongf compressedSize = compressBound(data.size());
    vector<char> compressedData(compressedSize);
    
    if (compress(reinterpret_cast<Bytef*>(compressedData.data()), &compressedSize, 
                reinterpret_cast<const Bytef*>(data.data()), data.size()) != Z_OK) {
        throw runtime_error("Compression failed");
    }
    compressedData.resize(compressedSize);
    return compressedData;
}

uint32_t current_time() {
    return static_cast<uint32_t>(time(nullptr));
}

string createBlob(const string& content) {
    write_blob(content); // Call your write_blob function
    string hash = compute_sha1(content); // Get the SHA-1 of the original content
    return hash; // Return the hash for use in the index file
}

void writeIndexFile(const vector<string>& files) {
    // Check if the index file exists
    ifstream existingIndexFile(".git/index", ios::binary);
    bool append = existingIndexFile.is_open();
    
    ofstream indexFile(".git/index", ios::binary);  // Overwrite to rewrite with updated entries
    stringstream fullContentBuffer;
    
    if (!indexFile.is_open()) {
        cerr << "Could not open index file for writing.\n";
        return;
    }

    uint32_t entryCount = 0;
    // if (append) {
    //     // Read the existing index content into the buffer, up to the start of entries
    //     fullContentBuffer << existingIndexFile.rdbuf();
    //     // Get the entry count from the existing index
    //     existingIndexFile.seekg(8, ios::beg);
    //     existingIndexFile.read(reinterpret_cast<char*>(&entryCount), 4);
    //     entryCount = ntohl(entryCount);
    //     cout<<"\n inside index";
    //     existingIndexFile.close();
    // } else {
        // Write header for a new index file
        fullContentBuffer.write("DIRC", 4);
        uint32_t version = htonl(2);
        fullContentBuffer.write(reinterpret_cast<char*>(&version), 4);
    // }

    entryCount += files.size();
    uint32_t entryCountNetworkOrder = htonl(entryCount);
    fullContentBuffer.seekp(8);  // Position after "DIRC" and version
    fullContentBuffer.write(reinterpret_cast<char*>(&entryCountNetworkOrder), 4);

    for (const string& file : files) {
        struct stat st;
        if (stat(file.c_str(), &st) == -1) {
            cerr << "Failed to stat file: " << file << "\n";
            continue;
        }
        ifstream fileStream(file, ios::binary);
        stringstream fileBuffer;
        fileBuffer << fileStream.rdbuf();
        string content = fileBuffer.str();
        string contentHash = createBlob(content);
        string contentHashBinary = hex_to_bytes(contentHash);
        if (contentHashBinary.size() != 20) {
            cerr << "SHA-1 hash size mismatch.\n";
            return;
        }

        uint32_t ctime_sec = htonl(static_cast<uint32_t>(st.st_ctime));
        uint32_t ctime_nsec = htonl(static_cast<uint32_t>(st.st_ctim.tv_nsec));
        fullContentBuffer.write(reinterpret_cast<char*>(&ctime_sec), 4);
        fullContentBuffer.write(reinterpret_cast<char*>(&ctime_nsec), 4);
        uint32_t mtime_sec = htonl(static_cast<uint32_t>(st.st_mtime));
        uint32_t mtime_nsec = htonl(static_cast<uint32_t>(st.st_mtim.tv_nsec));
        fullContentBuffer.write(reinterpret_cast<char*>(&mtime_sec), 4);
        fullContentBuffer.write(reinterpret_cast<char*>(&mtime_nsec), 4);
        uint32_t dev = htonl(static_cast<uint32_t>(st.st_dev));
        fullContentBuffer.write(reinterpret_cast<char*>(&dev), 4);
        uint32_t ino = htonl(static_cast<uint32_t>(st.st_ino));
        fullContentBuffer.write(reinterpret_cast<char*>(&ino), 4);
        uint32_t mode = htonl(static_cast<uint32_t>(st.st_mode));
        fullContentBuffer.write(reinterpret_cast<char*>(&mode), 4);
        uint32_t uid = htonl(static_cast<uint32_t>(st.st_uid));
        fullContentBuffer.write(reinterpret_cast<char*>(&uid), 4);
        uint32_t gid = htonl(static_cast<uint32_t>(st.st_gid));
        fullContentBuffer.write(reinterpret_cast<char*>(&gid), 4);
        uint32_t file_size = htonl(static_cast<uint32_t>(st.st_size));
        fullContentBuffer.write(reinterpret_cast<char*>(&file_size), 4);

        fullContentBuffer.write(contentHashBinary.c_str(), 20);
        fullContentBuffer.write(file.c_str(), file.size() + 1);

        int padding = (8 - ((62 + file.size() + 1) % 8)) % 8;
        for (int i = 0; i < padding; ++i) {
            fullContentBuffer.put('\0');
        }
    }

    string fullContent = fullContentBuffer.str();
    string checksum = calculate_sha1(fullContent);
    string checksumBinary = hex_to_bytes(checksum);
    if (checksumBinary.size() != 20) {
        cerr << "Checksum hash size mismatch.\n";
        return;
    }
    fullContentBuffer.write(checksumBinary.c_str(), 20);

    indexFile.write(fullContentBuffer.str().c_str(), fullContentBuffer.str().size());
    indexFile.close();
}




string currentTimestamp() {
    time_t now = time(nullptr);
    tm* ptm = gmtime(&now);
    char buffer[32];
    strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", ptm);
    return buffer;
}
string getCurrentTreeSHA() {

    return "your_tree_sha_here"; // Placeholder
}


string getParentCommitSHA() {
    ifstream headFile(".git/HEAD");
    if (headFile.is_open()) {
        string parentSHA;
        getline(headFile, parentSHA);
        headFile.close();
        return parentSHA;
    }
    return "";
}






string bytes_to_hex(const string& bytes) {
    ostringstream hexStream;
    for (unsigned char byte : bytes) {
        hexStream << setw(2) << setfill('0') << hex << static_cast<int>(byte);
    }
    return hexStream.str();
}

vector<IndexEntry> getIndexEntries() {
    vector<IndexEntry> entries;
    ifstream indexFile(".git/index", ios::binary);
    
    if (!indexFile.is_open()) {
        cerr << "Failed to open index file.\n";
        return entries;
    }


    indexFile.seekg(12, ios::beg);

    while (true) {
        IndexEntry entry;
        

        char modeBuffer[6] = {0};
        indexFile.read(modeBuffer, 6);
        if (indexFile.gcount() != 6) break; 
        entry.mode = string(modeBuffer, 6);

        getline(indexFile, entry.filename, '\0');

        char sha1Buffer[20] = {0};
        indexFile.read(sha1Buffer, 20);
        if (indexFile.gcount() != 20) {
            cerr << "Error reading SHA-1 hash in index file.\n";
            break;
        }
        entry.sha1 = bytes_to_hex(string(sha1Buffer, 20));

        entries.push_back(entry);
    }

    indexFile.close();
    return entries;
}

string writeTreeObject(const vector<IndexEntry>& entries) {
    stringstream treeContent;


    vector<IndexEntry> sortedEntries = entries;
    sort(sortedEntries.begin(), sortedEntries.end(), [](const IndexEntry& a, const IndexEntry& b) {
        return a.filename < b.filename;
    });


    for (const auto& entry : sortedEntries) {
        treeContent << entry.mode << " " << entry.filename << '\0';
        string sha1Binary = hex_to_bytes(entry.sha1);
        treeContent.write(sha1Binary.data(), sha1Binary.size());
    }


    string treeData = treeContent.str();
    string header = "tree " + to_string(treeData.size()) + '\0';
    string fullData = header + treeData;

    string treeSHA = compute_sha1(fullData);

    string dir = treeSHA.substr(0, 2);
    string file = treeSHA.substr(2);
    string treePath = "./.git/objects/" + dir + "/" + file;
    fs::create_directories("./.git/objects/" + dir);

    uLongf compressedSize = compressBound(fullData.size());
    vector<Bytef> compressedData(compressedSize);
    if (compress(compressedData.data(), &compressedSize, reinterpret_cast<const Bytef*>(fullData.data()), fullData.size()) != Z_OK) {
        cerr << "Failed to compress tree data.\n";
        return "";
    }

    ofstream treeFile(treePath, ios::binary);
    if (treeFile.is_open()) {
        treeFile.write(reinterpret_cast<const char*>(compressedData.data()), compressedSize);
        treeFile.close();
    } else {
        cerr << "Failed to write tree object.\n";
        return "";
    }

    return treeSHA;
}

string createTreeObject(const vector<TreeEntry>& treeEntries) {
    stringstream treeContent;
    
    for (const auto& entry : treeEntries) {
        treeContent << entry.mode << " " << entry.name << '\0';
        treeContent.write(entry.sha1.c_str(), 20);  // SHA-1 hash in binary format
    }
    
    string treeSHA = calculate_sha1(treeContent.str());
    
    
    string treePath = ".git/objects/" + treeSHA;
    ofstream treeFile(treePath, ios::binary);
    if (treeFile.is_open()) {
        treeFile.write(treeContent.str().c_str(), treeContent.str().size());
        treeFile.close();
    }

    return treeSHA;
}
void updateBranchReference(const string& commitHash) {
    ofstream branchFile(".git/refs/heads/main", ios::trunc);
    if (branchFile.is_open()) {
        branchFile << commitHash;
        branchFile.close();
    } else {
        cerr << "Failed to update branch reference.\n";
    }
}

void appendToMasterLog(const string& parentSHA, const string& thisSHA, const string& author, const string& email, const string& commitMsg) {
    
    ofstream logFile(".git/logs/refs/heads/master", ios::app);
    if (!logFile.is_open()) {
        cerr << "Failed to open master log file.\n";
        return;
    }
    time_t now = time(nullptr);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    logFile << parentSHA << " " << thisSHA << " " << author << " <" << email << "> " << timestamp
            << " commit: " << commitMsg << "\n";
    logFile.close();
}

string getLastCommitSHA() {
    ifstream logFile(".git/logs/refs/heads/master");
    if (!logFile.is_open()) {
        return string(40, '0');  // No parent, return a 40 '0's string
    }

    string lastLine, line;
    while (getline(logFile, line)) {
        lastLine = line;  // Store the last line read
    }
    logFile.close();

    if (!lastLine.empty()) {
        istringstream iss(lastLine);
        string prevParentSHA, prevCommitSHA;
        iss >> prevParentSHA >> prevCommitSHA;
        return prevCommitSHA;
    }
    return string(40, '0');  // No parent, return a 40 '0's string
}

string writeCommitObject(const string& treeSHA, const string& parentSHA, const string& message) {
    stringstream commitContent;
    commitContent << "tree " << treeSHA << "\n";
    if (!parentSHA.empty()) {
        commitContent << "parent " << parentSHA << "\n";
    }
    commitContent << " Someone " << currentTimestamp() << " +0530\n";
    commitContent << " <some@example.com> " << currentTimestamp() << " +0530\n\n";
    commitContent << message << "\n";
    
    
    
    
    string commitData = commitContent.str();
    string commitSHA = calculate_sha1(commitData);

    cout<<"\n debug:"<<commitData<<'\n';

    string commitPath = ".git/objects/" + commitSHA.substr(0, 2) + "/" + commitSHA.substr(2);
    fs::create_directories(".git/objects/" + commitSHA.substr(0, 2));
    ofstream commitFile(commitPath, ios::binary);
    if (commitFile.is_open()) {
        commitFile.write(commitData.c_str(), commitData.size());
        commitFile.close();
    } else {
        cerr << "Failed to write commit object.\n";
        return "";
    }

    return commitSHA;
}

void updateHead(const string& commitSHA) {
    ofstream headFile(".git/HEAD", ios::trunc);
    if (headFile.is_open()) {
        headFile << commitSHA;
        headFile.close();
    } else {
        cerr << "Failed to update HEAD.\n";
    }

    ofstream branchFile(".git/refs/heads/main", ios::trunc);
    if (branchFile.is_open()) {
        branchFile << commitSHA;
        branchFile.close();
    } else {
        cerr << "Failed to update branch reference.\n";
    }
}

void commit(const string& message) {
    vector<IndexEntry> entries = getIndexEntries();
    if (entries.empty()) {
        cerr << "No staged changes to commit.\n";
        return;
    }

    string treeSHA = writeTreeObject(entries);
    if (treeSHA.empty()) {
        cerr << "Failed to create tree object.\n";
        return;
    }

    string parentSHA = getParentCommitSHA();
    if(parentSHA=="ref: refs/heads/main"){
        parentSHA = "0000000000000000000000000000000000000000";
    }
    string commitSHA = writeCommitObject(treeSHA, parentSHA, message);
    if (commitSHA.empty()) {
        cerr << "Failed to create commit object.\n";
        return;
    }

    updateHead(commitSHA);
    cout << "Commit created: " << commitSHA << "\n";

    string author = "someone";
    string email = "someone@example.com";
    appendToMasterLog(parentSHA, commitSHA, author, email, message);

    // Remove the index file after a successful commit
    string indexPath = ".git/index";
    if (fs::exists(indexPath)) {
        fs::remove(indexPath);
        //cout << "Index file deleted successfully.\n";
    }
}




vector<CommitLogEntry> parseCommitLogFile(const string& filePath) {
    ifstream logFile(filePath);
    vector<CommitLogEntry> commitLogEntries;
    string line;

    if (!logFile.is_open()) {
        cerr << "Could not open commit log file.\n";
        return commitLogEntries;
    }

    while (getline(logFile, line)) {
        istringstream iss(line);
        CommitLogEntry entry;
        string author, email, date, time, tz;


        iss >> entry.parentSHA >> entry.currentSHA >> entry.authorName >> entry.authorEmail
            >> date >> time;


        entry.timestamp = date + " " + time +" +0530";


        string temp;
        iss >> temp; 
        getline(iss, entry.commitMessage); 
        entry.commitMessage = entry.commitMessage.substr(1);

        // Append parsed entry to vector
        commitLogEntries.push_back(entry);
    }

    logFile.close();
    return commitLogEntries;
}

void displayCommitLog() {
    string logFilePath = ".git/logs/refs/heads/master";
    vector<CommitLogEntry> commitLogEntries = parseCommitLogFile(logFilePath);

    if (commitLogEntries.empty()) {
        cout << "No commits found in the log.\n";
        return;
    }

    for (auto it = commitLogEntries.rbegin(); it != commitLogEntries.rend(); ++it) {
        const auto& entry = *it;
        cout << "Commit: " << entry.currentSHA << "\n";
        cout << "Parent: " << (entry.parentSHA.find("ref:") == 0 ? "0000000000000000000000000000000000000000" : entry.parentSHA)<< "\n";
        cout << "Author: " << entry.authorName << "\nEmail: " << entry.authorEmail << "\n";
        cout << "Date: " << entry.timestamp << "\n";
        cout << "Message: " << entry.commitMessage << "\n\n";
    }
}





////////////////////////////////////////////////////////////////////////////////////////////////////
struct commitEntry {
    string tree;
    string parent;
    string author;
    string email;
    string date;
    string message;
};


string readObjectBySHA(const string& sha) {
    string path = ".git/objects/" + sha.substr(0, 2) + "/" + sha.substr(2);
    cout<<"\ndebug readobj:"<<path;
    ifstream file(path, ios::binary);
    if (!file.is_open()) {
        cerr << "Error: Could not open object file for SHA " << sha << "\n";
        return "";
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
bool isValidSHA1(const string& sha1) {
    // Check if the SHA-1 is exactly 40 characters long
    if (sha1.size() != 40) {
        return false;
    }

    // Check if all characters are hexadecimal
    for (char c : sha1) {
        if (!isxdigit(c)) {
            return false;
        }
    }

    return true;
}

vector<commitEntry> parseIndexFile(const string& indexFileContent) {
    vector<commitEntry> entries;
    istringstream stream(indexFileContent);
    commitEntry entry;

    string line;
    while (getline(stream, line)) {
        // Skip empty lines
        if (line.empty()) {
            continue;
        }


        cout << "Parsing line: " << line << endl;


        if (line.find("tree ") == 0) {
            if (line.size() > 5) {  
                entry.tree = line.substr(5);
            } else {
                cerr << "Warning: Unexpected line format for tree entry: " << line << "\n";
                continue;
            }
        } else if (line.find("parent ") == 0) {
            if (line.size() > 7) {  
                entry.parent = line.substr(7);
            } else {
                cerr << "Warning: Unexpected line format for parent entry: " << line << "\n";
                continue;
            }
        } else if (line.find('@') != string::npos) {
            istringstream iss(line);
            iss >> entry.author >> entry.email;
        } else if (line.find('+') != string::npos) {
            
            entry.date = line;
        } else if (!line.empty()) {
            
            entry.message += line + "\n";
        }

        
        if (stream.peek() == 'c' || stream.eof()) {
            entries.push_back(entry);
            entry = commitEntry(); 
        }
    }
    return entries;
}



struct fileEntry {
    string filename;
    string sha1;
};


vector<fileEntry> parseTree(const string& treeContent) {
    vector<fileEntry> entries;
    istringstream stream(treeContent);
    string line;


    while (getline(stream, line)) {
        istringstream lineStream(line);
        fileEntry entry;

        string mode;
        lineStream >> mode;


        string sha1Binary(20, '\0');
        lineStream.read(&sha1Binary[0], 20);
        entry.sha1 = bytes_to_hex(sha1Binary); 

        lineStream >> entry.filename;

        entries.push_back(entry);
    }

    return entries;
}

vector<fileEntry> getFilesFromTree(const string& treeSHA) {
    vector<fileEntry> files;

    string treeContent = readObjectBySHA(treeSHA);
    if (treeContent.empty()) {
        cerr << "Error: Could not read tree object for SHA " << treeSHA << "\n";
        return files;
    }

    istringstream stream(treeContent);
    while (stream) {
        uint32_t mode;
        stream.read(reinterpret_cast<char*>(&mode), sizeof(mode));

        char sha1[20];
        stream.read(sha1, sizeof(sha1));

        string filename;
        getline(stream, filename, '\0');

        if (stream.eof() || stream.fail()) {
            break;
        }

        fileEntry file;
        file.filename = filename;
        file.sha1 = string(sha1, sizeof(sha1));  // SHA-1 as a string

        
        files.push_back(file);

        
        int entrySize = 4 + 20 + filename.size() + 1;  // mode + sha1 + filename + null terminator
        int padding = (8 - (entrySize % 8)) % 8; // Calculate padding
        stream.ignore(padding); // Ignore padding bytes
    }

    return files;
}

void restoreFiles(const vector<fileEntry>& entries) {
    for (const auto& entry : entries) {
        string content = readObjectBySHA(entry.sha1);
        if (content.empty()) {
            cerr << "Warning: Could not retrieve content for file " << entry.filename << " (SHA: " << entry.sha1 << ")\n";
            continue;
        }

        fs::path filePath = entry.filename;

        
        if (!filePath.parent_path().empty()) {
            fs::create_directories(filePath.parent_path());
        }

        try {
        
            if (fs::exists(filePath)) {
                fs::remove(filePath);
            }

        
            ofstream outFile(filePath, ios::binary);
            if (outFile) {
                outFile << content;
                outFile.close();
                cout << "Restored file: " << entry.filename << "\n";
            } else {
                cerr << "Error: Could not open file for writing: " << entry.filename << "\n";
            }
        } catch (const fs::filesystem_error& e) {
            cerr << "Filesystem error for file " << entry.filename << ": " << e.what() << "\n";
        }
    }
}





int main(int argc, char *argv[])
{
    if (argc < 2) {
        cerr<<"No command provided.\n";
        return EXIT_FAILURE;
    }

    string command = argv[1];

    if (command == "init") {
    try {
        bool is_reinitialized = false;

        if (fs::exists(".git")) {
            is_reinitialized = true;
        }
        fs::create_directory(".git");
        fs::create_directory(".git/objects");
        fs::create_directory(".git/refs");
        fs::create_directory(".git/logs");
        fs::create_directory(".git/refs/heads");
        fs::create_directory(".git/refs/tags");
        fs::create_directory(".git/logs/refs");
        fs::create_directory(".git/logs/refs/heads");
        fs::create_directory(".git/logs/refs/tags");

        if (!fs::exists(".git/HEAD")) {
            ofstream headFile(".git/HEAD");
            if (headFile.is_open()) {
                headFile << "ref: refs/heads/main\n";
                headFile.close();
            } else {
                cerr << "Failed to create .git/HEAD file.\n";
                return EXIT_FAILURE;
            }
        }

        if (is_reinitialized) {
            cout << "Reinitialized git directory\n";
        } else {
            cout << "Initialized git directory\n";
        }
    } catch (const fs::filesystem_error& e) {
        cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    }
    else if (command == "cat-file") {
        if (argc < 3) {
            cout << "Incomplete command. Expected: ./mygit cat-file <flag> <hash>\n";
            return EXIT_FAILURE;
        }
        
        string flag = argv[2]; 
        string hash = argv[3]; 
        string dir = hash.substr(0, 2);
        string blob_sha = hash.substr(2);
        string path = "./.git/objects/" + dir + "/" + blob_sha;

        ifstream file(path, ios::binary);
        if (!file.is_open()) {
            cout << "Can't open object file\n";
            return EXIT_FAILURE;
        }

        vector<char> compressedData((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();

        
        z_stream strm = {};
        strm.next_in = reinterpret_cast<Bytef*>(compressedData.data());
        strm.avail_in = compressedData.size();

        vector<char> decompressedData(4096);
        strm.next_out = reinterpret_cast<Bytef*>(decompressedData.data());
        strm.avail_out = decompressedData.size();

        if (inflateInit(&strm) != Z_OK) {
            cerr << "Failed to initialize zlib.\n";
            return EXIT_FAILURE;
        }

        string object_str;
        int ret;
        do {
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR || ret == Z_MEM_ERROR || ret == Z_DATA_ERROR) {
                cerr << "Zlib decompression failed.\nerror:"<<ret<<'\n';
                inflateEnd(&strm);
                return EXIT_FAILURE;
            }
            object_str.append(decompressedData.data(), decompressedData.size() - strm.avail_out);
            strm.next_out = reinterpret_cast<Bytef*>(decompressedData.data());
            strm.avail_out = decompressedData.size();
        } while (ret != Z_STREAM_END);

        inflateEnd(&strm);
        long long pos = object_str.find('\0');
        if (pos == string::npos) {
            cerr << "Failed to parse object content.\n";
            return EXIT_FAILURE;
        }

        string object_content = object_str.substr(pos + 1);
        size_t object_size = object_str.size();
        string object_type = "blob"; 

        // Handle flags
        if (flag == "-p") {
            cout << object_content;
        } else if (flag == "-s") {
            cout << object_size << " bytes\n";
        } else if (flag == "-t") {
            cout << object_type << "\n";
        } else {
            cout << "Unknown flag: " << flag << "\n";
            return EXIT_FAILURE;
        }
    }else if (command == "hash-object") {
        bool writeFlag = false;
        string filename;

        for (int i = 2; i < argc; ++i) {
            if (string(argv[i]) == "-w") {
                writeFlag = true;
            } else {
                filename = argv[i];
            }
        }

        ifstream file(filename, ios::binary);
        if (!file.is_open()) {
            cerr<<"Cannot open file: "<<filename<<endl;
            return EXIT_FAILURE;
        }
        stringstream buffer;
        buffer<<file.rdbuf();
        string fileContent = buffer.str();
        file.close();

        string header = "blob " + to_string(fileContent.size()) + '\0';
        string gitObjectContent = header + fileContent;
        string hash = calculate_sha1(gitObjectContent);

        cout<<hash;

        if (writeFlag) {
            write_object(hash, gitObjectContent);
        }

    }else if(command== "write-tree"){
        if (argc < 2) {
        cerr << "Usage: ./your_program <command>\n";
        return EXIT_FAILURE;
    }
        write_tree_command();
    }else if(command == "ls-tree"){
        bool name_only = false;
        string tree_sha;

        if (string(argv[2]) == "--name-only") {
            name_only = true;
            tree_sha = argv[3];
        } else {
            tree_sha = argv[2];
        }
        ls_tree(tree_sha, name_only);

        
    }else if (command == "add") {
        if (argc < 3) {
            cerr << "Usage: ./mygit add <file1> <file2> ... or ./mygit add .\n";
            return 1;
        }
        vector<string> filesToStage;
        if (string(argv[2]) == ".") {
            for (const auto& entry : fs::recursive_directory_iterator(".")) {
                if (fs::is_regular_file(entry.path()) && 
                    entry.path().string().find(".git") == string::npos && entry.path().string().find("mygit") == string::npos && entry.path().string().find("mygit.cpp") == string::npos) {
                    filesToStage.push_back(entry.path().string());
                    cout<<"\nfile: "<<entry.path().string();
                }
            }
        }
        else {
            for (int i = 2; i < argc; ++i) {
            fs::path path = argv[i];

            if (fs::is_directory(path)) {
                for (const auto& entry : fs::recursive_directory_iterator(path)) {
                    if (fs::is_regular_file(entry.path()) &&
                        entry.path().string().find(".git") == string::npos&& entry.path().string().find("mygit") == string::npos && entry.path().string().find("mygit.cpp") == string::npos) {
                        
                        string relativePath = fs::relative(entry.path(), fs::current_path()).string();
                        filesToStage.push_back(relativePath);
                    }
                }
            } else if (fs::is_regular_file(path)) {
                string relativePath = fs::relative(path, fs::current_path()).string();
                filesToStage.push_back(relativePath);
                // cout << "\nfile: " << relativePath;
            } else {
                cerr << "Warning: " << path.string() << " is not a valid file or directory.\n";
            }
        }
        }
        writeIndexFile(filesToStage);
    }else if(command =="commit"){
        if (argc >= 4 && string(argv[2]) == "-m") {
            string message = argv[3];
            commit(message);
        } else {
            // cerr << "Usage: ./mygit commit -m \"Commit message\"\n";
            // return 1;
            commit("commit with no mssg");
        }
    }else if(command == "log"){
        displayCommitLog();
    }else if(command == "checkout"){
        if(argc!=3){
            cerr << "Usage: ./mygit checkout <commit_sha>\n";
            return 1;
        }
        string indexSHA = argv[2];
        cout<<"\ndebug:"<<indexSHA;
        string indexContent = readObjectBySHA(indexSHA);

        if (indexContent.empty()) {
            cerr << "Error: Invalid index SHA or index file not found.\n";
            return 1;
        }

        cout<<"\ndebug indexcontent:"<<indexContent;
        // Parse index file and restore files
        auto entries = parseIndexFile(indexContent);
        vector<fileEntry> fileEntries;

    for (const auto& entry : entries) {
        vector<fileEntry> filesFromTree = getFilesFromTree(entry.tree);
        for (const auto& file : filesFromTree) {
            fileEntries.push_back(file);
        }
    }

    restoreFiles(fileEntries);



        cout << "Checked out index: " << indexSHA << "\n";
    }else {
        cerr<<"Unknown command "<<command<<'\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
