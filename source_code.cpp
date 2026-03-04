#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

// Gia lap RAM rat nho: Chi chua duoc toi da 3 so thuc (double)
const int RAM_LIMIT = 3;

// Ham in mang de hien thi truc quan
void printBuffer(const vector<double>& arr) {
    cout << "[ ";
    for (double val : arr) cout << val << " ";
    cout << "]";
}

// PHA 1: DOC TUNG PHAN VUA RAM, SAP XEP VA GHI RA FILE TAM
int createSortedChunks(const string& inputFile) {
    ifstream inFile(inputFile, ios::binary);
    if (!inFile) {
        cout << "Khong the mo file goc!\n";
        return 0;
    }

    vector<double> ramBuffer(RAM_LIMIT);
    int chunkCount = 0;

    cout << "\n=== PHA 1: CHIA NHO FILE GOC VA SAP XEP TRONG RAM ===\n";
    cout << "Luat: RAM chi chua duoc toi da " << RAM_LIMIT << " so.\n";

    while (inFile) {
        // Doc du lieu tu file vao RAM cho den khi day RAM
        inFile.read(reinterpret_cast<char*>(ramBuffer.data()), RAM_LIMIT * sizeof(double));
        int bytesRead = inFile.gcount();
        int elementsRead = bytesRead / sizeof(double);

        if (elementsRead == 0) break; // Da doc het file

        // Thu gon RAM neu phan cuoi file khong du 3 so
        ramBuffer.resize(elementsRead);

        // Sap xep du lieu dang co trong RAM
        sort(ramBuffer.begin(), ramBuffer.end());

        // Ghi ket qua trong RAM ra mot file tam (chunk)
        string chunkName = "chunk_" + to_string(chunkCount) + ".bin";
        ofstream outFile(chunkName, ios::binary);
        outFile.write(reinterpret_cast<const char*>(ramBuffer.data()), bytesRead);
        outFile.close();

        cout << "-> Doc " << elementsRead << " so vao RAM, sap xep: ";
        printBuffer(ramBuffer);
        cout << " => Ghi ra file tam: " << chunkName << "\n";

        chunkCount++;
        ramBuffer.resize(RAM_LIMIT); // Xoa RAM de chuan bi cho luot doc tiep theo
    }

    inFile.close();
    return chunkCount; // Tra ve tong so file tam da tao
}

// PHA 2: TRON CAC FILE TAM THANH 1 FILE HOAN CHINH
void mergeChunks(int numChunks, const string& outputFile) {
    cout << "\n=== PHA 2: TRON (MERGE) CAC FILE TAM THANH KET QUA ===\n";
    cout << "Luat: Chi doc tung so mot tu cac file tam len RAM de so sanh.\n";

    vector<ifstream> chunkFiles(numChunks);
    vector<double> topValues(numChunks); // Luu gia tri dang dung o dau moi file tam
    vector<bool> fileActive(numChunks, true); // Kiem tra file nao da doc het
    int activeCount = numChunks;

    // Mo tat ca file tam va doc len 1 so dau tien cua moi file
    for (int i = 0; i < numChunks; ++i) {
        string chunkName = "chunk_" + to_string(i) + ".bin";
        chunkFiles[i].open(chunkName, ios::binary);
        chunkFiles[i].read(reinterpret_cast<char*>(&topValues[i]), sizeof(double));

        if (chunkFiles[i].gcount() == 0) {
            fileActive[i] = false;
            activeCount--;
        }
    }

    ofstream outFile(outputFile, ios::binary);
    int step = 1;

    // Bat dau vong lap tron du lieu
    while (activeCount > 0) {
        int minIndex = -1;
        double minValue = 1e9; // Khoi tao mot so cuc lon

        // Quet qua cac so dang nam tren RAM de tim so nho nhat
        for (int i = 0; i < numChunks; ++i) {
            if (fileActive[i] && topValues[i] < minValue) {
                minValue = topValues[i];
                minIndex = i;
            }
        }

        // Ghi so nho nhat tim duoc vao file dich
        outFile.write(reinterpret_cast<const char*>(&minValue), sizeof(double));
        cout << "Buoc " << step++ << ": Lay so nho nhat (" << minValue
             << ") tu file " << "chunk_" << minIndex << " ghi vao ket qua.\n";

        // Sau khi lay so o file nao, ta doc tiep 1 so cua file do len RAM
        chunkFiles[minIndex].read(reinterpret_cast<char*>(&topValues[minIndex]), sizeof(double));

        // Neu file do da het du lieu
        if (chunkFiles[minIndex].gcount() == 0) {
            fileActive[minIndex] = false;
            activeCount--;
            cout << "   [INFO] File chunk_" << minIndex << " da doc het du lieu.\n";
        }
    }

    outFile.close();

    // Dong va xoa ranh cac file tam tren o cung
    for (int i = 0; i < numChunks; ++i) {
        chunkFiles[i].close();
        remove(("chunk_" + to_string(i) + ".bin").c_str());
    }
    cout << "-> Hoan tat! Da xoa cac file tam va luu ket qua vao " << outputFile << "\n";
}

int main() {
    string inputFile = "du_lieu_goc.bin";
    string outputFile = "du_lieu_da_sap_xep.bin";
    int n;

    cout << "==== CHUONG TRINH MINH HOA SAP XEP NGOAI (EXTERNAL SORT) ====\n";
    cout << "Nhap so luong phan tu ban muon sap xep: ";
    cin >> n;

    if (n <= 0) {
        cout << "So luong phan tu phai lon hon 0. Ket thuc chuong trinh.\n";
        return 0;
    }

    // Ghi du lieu nguoi dung nhap vao file goc
    ofstream outInput(inputFile, ios::binary);
    if (!outInput) {
        cout << "Khong the tao file goc!\n";
        return 0;
    }

    cout << "Vui long nhap " << n << " so thuc (cach nhau boi khoang trang hoac Enter):\n";
    for (int i = 0; i < n; ++i) {
        double val;
        cin >> val;
        outInput.write(reinterpret_cast<const char*>(&val), sizeof(double));
    }
    outInput.close();

    cout << "\nDa luu " << n << " so vao file goc thanh cong.\n";

    // Thuc thi thuat toan External Sort
    int numChunks = createSortedChunks(inputFile);
    if (numChunks > 0) {
        mergeChunks(numChunks, outputFile);
    }

    return 0;
}
