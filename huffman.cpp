#include <bits/stdc++.h>
#include <sys/time.h>
using namespace std;

struct node
{

    // One of the input characters
    int ascii;

    // Frequency of the character
    int freq;
    bool real;

    // Left and right child
    node *left, *right;

    node(int ascii, int freq, bool r)
    {
        left = right = NULL;
        real = r;
        this->ascii = ascii;
        this->freq = freq;
    }
};

struct compare
{

    bool operator()(node* l, node* r)
    {
        return (l->freq > r->freq);
    }
};



vector<char> readFile(string filename)
{
    ifstream  instream(filename, ios::in);
    vector<char> input(istreambuf_iterator<char>(instream), (istreambuf_iterator<char>()));
    instream.close();
    return input;
}

map<int, int> getFreqMap(vector<char> &input)
{
    map<int, int> freqMap;
    int ascii;
    for(int i=0; i < input.size(); i++)
    {
        ascii = input[i];
        freqMap[ascii] ++;
    }

    return freqMap;
}

void traverseHuffman(struct node* root, string code, map<int, string> &encoding)
{
    if (!root)
        return;

    if (root->real)
    {
        encoding.insert(pair<int, string>(int(root->ascii), code));
    }

    traverseHuffman(root->left, code + "0", encoding);
    traverseHuffman(root->right, code + "1", encoding);
}


map<int, string> buildHuffman(map<int, int> &freq_map)
{
    struct node *left, *right, *sum;
    priority_queue<node*, vector<node*>, compare> minHeap;
    int unique_char = '$';
    for (auto freq : freq_map)
    {
        minHeap.push(new node(freq.first, freq.second, true));
    }

    while (minHeap.size() > 1)
    {

        // Extract the two minimum
        // freq items from min heap
        left = minHeap.top();
        minHeap.pop();

        right = minHeap.top();
        minHeap.pop();

        // Create a new internal node with
        // frequency equal to the sum of the
        // two nodes frequencies. Make the
        // two extracted node as left and right children
        // of this new node. Add this node
        // to the min heap '$' is a special value
        // for internal nodes, not used
        sum = new node(unique_char, left->freq + right->freq, false);

        sum->left = left;
        sum->right = right;

        minHeap.push(sum);
    }
    map<int, string> char_encoded;
    traverseHuffman(minHeap.top(),"", char_encoded);
    return char_encoded;
}

pair<vector<unsigned char>, int> compress(vector<char> &input, map<int, string> &codes)
{
    vector<unsigned char> compressed;
    int paddedZeros = 0;
    string s = "";
    for(int i = 0; i<input.size(); i++)
    {
        s += codes[input[i]];
        if(i == input.size()-1 && s.size()%8 != 0)
        {
            paddedZeros = 8 - s.size()%8;
            s += string(paddedZeros, '0');
        }
        while(s.size() >= 8)
        {
            unsigned char coded = 0x00;
            int n = 0;
            while(n < 8)
            {
                unsigned char mask = (s[n]=='0')? 0x00 : 0x01;
                coded |= (mask << (7-n));
                n++;
            }
            compressed.push_back(coded);
            s.erase(0, 8);
        }
    }
    return {compressed, paddedZeros};
}

void display_codes(map<int, string> dict)
{

    cout << endl << "Huffman Tree:" << endl;
	cout << "Byte" << "\t"  << "Code" << "\t\t" << "New Code" << endl;
    for (auto &x : dict)
    {
        std::cout << (unsigned int)x.first   << "\t" << bitset<8>((unsigned int)x.first).to_string() << "\t" << x.second << endl ;
    }
}


void writeCompressedFile(pair<vector<unsigned char>, int> &compressData, map<int, string> &codes)
{
    ofstream output("compressed.txt");
    output << codes.size() << endl;
    for(auto &code : codes)
    {
        output << (unsigned char) code.first << code.second << endl;
    }
    output << compressData.second << endl; //padded zeros
    for(char c : compressData.first)
    {
        output << c;
    }

    display_codes(codes);
}
int getCompressedFileSizeInBytes(map<int, int> &freq_map, map<int, string> &codes)
{
    int size = 0;
    //header size
    size += log10(codes.size())+1;
    size ++; // for \n
    int totalFileBits = 0;
    for(auto &code : codes)
    {
        size ++; //one byte for the distinct character
        size += code.second.size(); //add bytes for code
        size ++; // for \n
        totalFileBits += freq_map[code.first]*code.second.size();
    }
    size ++; //one byte for padded zeros
    size ++; // for \n
    //add compressed file size
    size += ceil(((double)totalFileBits)/8.0);
    return size;
}
double getCompressionRatio(map<int, int> &freq_map, map<int, string> &codes, vector<char> &input)
{
    return (((double)input.size() / (double)getCompressedFileSizeInBytes(freq_map, codes)));
}
pair<vector<unsigned char>, pair<map<string, int>, int> > readCompressedFile(string file_name)
{
    map<string, int> codes;
    //check enoh valid

    ifstream  instream(file_name, ios_base::in | ios_base::binary);
    vector<unsigned char> buffer(istreambuf_iterator<char>(instream), {});

    //get header
    //get map size
    int bufferIndex = 0;
    string temp = "";
    while(bufferIndex<buffer.size() && buffer[bufferIndex]!='\n')
    {
        temp += buffer[bufferIndex++];
    }
    bufferIndex++; // for \n
    stringstream sstream(temp);
    int mapSize;
    sstream >> mapSize;
    //get map
    for(int i=0; i<mapSize; i++)
    {
        unsigned char distinctChar = buffer[bufferIndex++];
        temp = "";
        while(bufferIndex<buffer.size() && buffer[bufferIndex]!='\n')
        {
            temp += buffer[bufferIndex++];
        }
        bufferIndex++; // for \n
        codes[temp] = (int) distinctChar;
    }
    temp = "";
    temp += buffer[bufferIndex++];
    bufferIndex++;
    stringstream sstream2(temp);
    int paddedZeros;
    sstream2 >> paddedZeros;

    buffer.erase(buffer.begin(), buffer.begin()+bufferIndex);
	cout << endl << "Huffman Tree:" << endl;
	cout << "Byte" << "\t"  << "Code" << "\t\t" << "New Code" << endl;
    for(auto &f : codes){
        cout << f.second << "\t" << bitset<8>(f.second).to_string()  <<"\t " << f.first << endl;
    }
    return {buffer, {codes, paddedZeros}};
}

vector<unsigned char> get_decompressed_content( pair<vector<unsigned char>, pair<map<string, int>, int> > data)
{
    vector<unsigned char> buffer = data.first;
    pair<map<string, int>, int> item = data.second;
    int padded_zero = item.second;
    map<string, int> dict = item.first;
    string encoded;

    for(int i=0; i<buffer.size(); i++)
    {
        encoded += bitset<8>(buffer[i]).to_string() ;
    }

    int i =0;
    string code;
    vector<unsigned char> decompressed;
    while(i < encoded.size()-padded_zero)
    {
        while(dict.count(code) != 1 && i<encoded.size()-padded_zero)
        {
            code += encoded[i];
            i++;
        }
        if(dict.count(code) == 1)
        {
            decompressed.push_back((unsigned char)(dict.find(code)->second));
            code = "";
        }
    }

    return decompressed;
}


void writeDecompressedFile(vector<unsigned char> decomp)
{
    ofstream output("decompressed.txt");
    for(unsigned char c : decomp)
    {
        output << c;
    }
    output.close();
}


int main()
{
    clock_t start, fin;
    int choice;
    while(1){


    cout << "Please enter your choice number:" << endl;
    cout << "1- Compress" << endl << "2- Decompress" << endl ;
    cin >> choice;
    string file_name;

    cout << "Please enter file name:" << endl;
    cin >> file_name;

    start = clock();
    if(choice == 1)
    {
        vector<char> input = readFile(file_name);
        map<int, int> freqMap = getFreqMap(input);
        map<int, string> codes = buildHuffman(freqMap);
        double comp_ratio = getCompressionRatio(freqMap, codes, input);
        cout << "Compression ratio = "<< comp_ratio <<endl;
        if(comp_ratio < 1){
            cout << "Compression is not efficient !" << endl;
        }else{
        	auto compressData = compress(input, codes);
        	writeCompressedFile(compressData, codes);
	}
    }
    else if(choice == 2)
    {

        pair<vector<unsigned char>, pair<map<string, int>, int> > data = readCompressedFile(file_name);
        vector<unsigned char> decomp = get_decompressed_content(data);
        writeDecompressedFile(decomp);

    }else
    {
        cout << "Invalid Choice !" << endl;
    }
    fin = clock();
    double time_taken = double(fin - start) / double(CLOCKS_PER_SEC);
    cout << "Time taken by program is : " << fixed
         << time_taken << setprecision(5);
    cout << " sec " << endl;
    }
    return 0;
}
