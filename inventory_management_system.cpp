#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const string INVENTORY_FILE = "inventory.txt";
const string RECEIPT_FILE = "last_receipt.txt";
const int LOW_STOCK_LIMIT = 5;

string trim(const string& value) {
    const size_t start = value.find_first_not_of(" \t\r\n");
    if (start == string::npos) {
        return "";
    }
    const size_t finish = value.find_last_not_of(" \t\r\n");
    return value.substr(start, finish - start + 1);
}

string readText(const string& prompt) {
    string value;
    while (true) {
        cout << prompt;
        getline(cin, value);
        value = trim(value);
        if (!value.empty() && value.find('|') == string::npos) {
            return value;
        }
        cout << "Enter a non-empty value without the '|' character.\n";
    }
}

int readInteger(const string& prompt, int minimum, int maximum) {
    string input;
    int value;
    while (true) {
        cout << prompt;
        getline(cin, input);
        stringstream parser(input);
        char extra;
        if ((parser >> value) && !(parser >> extra) &&
            value >= minimum && value <= maximum) {
            return value;
        }
        cout << "Enter a whole number between " << minimum << " and " << maximum << ".\n";
    }
}

double readMoney(const string& prompt, double minimum, double maximum) {
    string input;
    double value;
    while (true) {
        cout << prompt;
        getline(cin, input);
        stringstream parser(input);
        char extra;
        if ((parser >> value) && !(parser >> extra) &&
            value >= minimum && value <= maximum) {
            return value;
        }
        cout << fixed << setprecision(2)
             << "Enter a value between " << minimum << " and " << maximum << ".\n";
    }
}

class Product {
protected:
    int id;
    string name;
    int quantity;
    double price;
    int reorderLevel;

public:
    Product(int productId, const string& productName, int productQuantity,
            double productPrice, int minimumStock)
        : id(productId), name(productName), quantity(productQuantity),
          price(productPrice), reorderLevel(minimumStock) {}

    virtual ~Product() = default;

    int getId() const { return id; }
    const string& getName() const { return name; }
    int getQuantity() const { return quantity; }
    double getPrice() const { return price; }
    int getReorderLevel() const { return reorderLevel; }

    void update(const string& newName, int newQuantity, double newPrice, int newReorderLevel) {
        name = newName;
        quantity = newQuantity;
        price = newPrice;
        reorderLevel = newReorderLevel;
    }

    bool isLowStock() const { return quantity <= reorderLevel; }
    bool canSell(int amount) const { return amount > 0 && amount <= quantity; }
    void removeQuantity(int amount) { quantity -= amount; }

    virtual string typeName() const = 0;
    virtual char typeCode() const = 0;
    virtual double sellingPrice() const { return price; }

    virtual string toFileLine() const {
        ostringstream row;
        row << typeCode() << '|' << id << '|' << name << '|' << quantity << '|'
            << fixed << setprecision(2) << price << '|' << reorderLevel;
        return row.str();
    }

    virtual void printExtra() const {}
};

class StandardProduct : public Product {
public:
    StandardProduct(int id, const string& name, int quantity, double price, int reorderLevel)
        : Product(id, name, quantity, price, reorderLevel) {}

    string typeName() const override { return "Standard"; }
    char typeCode() const override { return 'S'; }
};

class DiscountedProduct : public Product {
private:
    double discountPercent;

public:
    DiscountedProduct(int id, const string& name, int quantity, double price,
                      int reorderLevel, double discount)
        : Product(id, name, quantity, price, reorderLevel),
          discountPercent(discount) {}

    double getDiscountPercent() const { return discountPercent; }
    string typeName() const override { return "Discounted"; }
    char typeCode() const override { return 'D'; }
    double sellingPrice() const override { return price * (1.0 - discountPercent / 100.0); }

    string toFileLine() const override {
        ostringstream row;
        row << typeCode() << '|' << id << '|' << name << '|' << quantity << '|'
            << fixed << setprecision(2) << price << '|' << reorderLevel << '|'
            << discountPercent;
        return row.str();
    }

    void printExtra() const override {
        cout << "Discount     : " << fixed << setprecision(2)
             << discountPercent << "%\n";
    }
};

unique_ptr<Product> productFromLine(const string& line) {
    stringstream row(line);
    string type, idText, name, quantityText, priceText, reorderText, discountText;

    if (!getline(row, type, '|') || !getline(row, idText, '|') ||
        !getline(row, name, '|') || !getline(row, quantityText, '|') ||
        !getline(row, priceText, '|') || !getline(row, reorderText, '|')) {
        return nullptr;
    }

    try {
        const int id = stoi(idText);
        const int quantity = stoi(quantityText);
        const double price = stod(priceText);
        const int reorderLevel = stoi(reorderText);

        if (id <= 0 || name.empty() || quantity < 0 || price < 0 || reorderLevel < 0) {
            return nullptr;
        }

        if (type == "S") {
            return make_unique<StandardProduct>(id, name, quantity, price, reorderLevel);
        }

        if (type == "D" && getline(row, discountText)) {
            const double discount = stod(discountText);
            if (discount >= 0 && discount <= 100) {
                return make_unique<DiscountedProduct>(id, name, quantity, price,
                                                      reorderLevel, discount);
            }
        }
    } catch (const exception&) {
        return nullptr;
    }

    return nullptr;
}

class Inventory {
private:
    vector<unique_ptr<Product>> products;

    int findIndex(int productId) const {
        for (size_t index = 0; index < products.size(); ++index) {
            if (products[index]->getId() == productId) {
                return static_cast<int>(index);
            }
        }
        return -1;
    }

public:
    void load() {
        ifstream input(INVENTORY_FILE);
        string line;
        products.clear();

        while (getline(input, line)) {
            unique_ptr<Product> product = productFromLine(line);
            if (product != nullptr && findIndex(product->getId()) == -1) {
                products.push_back(move(product));
            }
        }
    }

    bool save() const {
        ofstream output(INVENTORY_FILE);
        if (!output) {
            return false;
        }
        for (const auto& product : products) {
            output << product->toFileLine() << '\n';
        }
        return true;
    }

    bool contains(int productId) const { return findIndex(productId) != -1; }

    bool add(unique_ptr<Product> product) {
        if (product == nullptr || contains(product->getId())) {
            return false;
        }
        products.push_back(move(product));
        return true;
    }

    bool update(int productId, const string& name, int quantity,
                double price, int reorderLevel) {
        const int index = findIndex(productId);
        if (index == -1) {
            return false;
        }
        products[index]->update(name, quantity, price, reorderLevel);
        return true;
    }

    bool remove(int productId) {
        const int index = findIndex(productId);
        if (index == -1) {
            return false;
        }
        products.erase(products.begin() + index);
        return true;
    }

    Product* get(int productId) {
        const int index = findIndex(productId);
        return index == -1 ? nullptr : products[index].get();
    }

    const Product* get(int productId) const {
        const int index = findIndex(productId);
        return index == -1 ? nullptr : products[index].get();
    }

    void display() const {
        cout << "\n--- Current Inventory ---\n";
        if (products.empty()) {
            cout << "No products are registered.\n";
            return;
        }

        cout << left << setw(8) << "ID" << setw(27) << "Name"
             << setw(15) << "Type" << right << setw(9) << "Qty"
             << setw(12) << "Price" << setw(12) << "Stock" << '\n';
        cout << string(83, '-') << '\n';

        for (const auto& product : products) {
            cout << left << setw(8) << product->getId()
                 << setw(27) << product->getName().substr(0, 25)
                 << setw(15) << product->typeName()
                 << right << setw(9) << product->getQuantity()
                 << setw(12) << fixed << setprecision(2) << product->sellingPrice()
                 << setw(12) << (product->isLowStock() ? "LOW" : "OK") << '\n';
        }
    }

    void search(const string& keyword) const {
        cout << "\n--- Search Results ---\n";
        bool found = false;
        for (const auto& product : products) {
            if (product->getName().find(keyword) != string::npos ||
                product->typeName().find(keyword) != string::npos) {
                cout << "ID: " << product->getId() << " | " << product->getName()
                     << " | Type: " << product->typeName()
                     << " | Quantity: " << product->getQuantity()
                     << " | Price: $" << fixed << setprecision(2)
                     << product->sellingPrice() << '\n';
                found = true;
            }
        }
        if (!found) {
            cout << "No product matched the search.\n";
        }
    }

    void reports() const {
        int availableStock = 0;
        double inventoryValue = 0;
        int lowStockProducts = 0;

        for (const auto& product : products) {
            availableStock += product->getQuantity();
            inventoryValue += product->getQuantity() * product->sellingPrice();
            if (product->isLowStock()) {
                ++lowStockProducts;
            }
        }

        cout << "\n--- Inventory Report ---\n"
             << "Total products : " << products.size() << '\n'
             << "Available stock: " << availableStock << " units\n"
             << "Inventory value: $" << fixed << setprecision(2) << inventoryValue << '\n'
             << "Low-stock items: " << lowStockProducts << '\n';
    }
};

void addProduct(Inventory& inventory) {
    cout << "\n--- Add Product ---\n";
    const int id = readInteger("Product ID: ", 1, 1000000);
    if (inventory.contains(id)) {
        cout << "A product with that ID already exists.\n";
        return;
    }

    const string name = readText("Product name: ");
    const int quantity = readInteger("Quantity: ", 0, 100000000);
    const double price = readMoney("Price: $", 0, 100000000);
    const int reorderLevel = readInteger("Low-stock threshold: ", 0, 100000000);
    const int type = readInteger("Product type (1=Standard, 2=Discounted): ", 1, 2);

    unique_ptr<Product> product;
    if (type == 1) {
        product = make_unique<StandardProduct>(id, name, quantity, price, reorderLevel);
    } else {
        const double discount = readMoney("Discount percentage: ", 0, 100);
        product = make_unique<DiscountedProduct>(id, name, quantity, price,
                                                 reorderLevel, discount);
    }

    inventory.add(move(product));
    inventory.save();
    cout << "Product added and inventory saved.\n";
}

void updateProduct(Inventory& inventory) {
    cout << "\n--- Update Product ---\n";
    const int id = readInteger("Product ID: ", 1, 1000000);
    if (!inventory.contains(id)) {
        cout << "No product was found with that ID.\n";
        return;
    }

    const string name = readText("New product name: ");
    const int quantity = readInteger("New quantity: ", 0, 100000000);
    const double price = readMoney("New price: $", 0, 100000000);
    const int reorderLevel = readInteger("New low-stock threshold: ", 0, 100000000);

    inventory.update(id, name, quantity, price, reorderLevel);
    inventory.save();
    cout << "Product updated and inventory saved.\n";
}

void deleteProduct(Inventory& inventory) {
    cout << "\n--- Delete Product ---\n";
    const int id = readInteger("Product ID: ", 1, 1000000);
    if (inventory.remove(id)) {
        inventory.save();
        cout << "Product deleted and inventory saved.\n";
    } else {
        cout << "No product was found with that ID.\n";
    }
}

void sellProduct(Inventory& inventory) {
    cout << "\n--- New Sale ---\n";
    const int id = readInteger("Product ID: ", 1, 1000000);
    Product* product = inventory.get(id);

    if (product == nullptr) {
        cout << "No product was found with that ID.\n";
        return;
    }

    const int amount = readInteger("Quantity to sell: ", 1, product->getQuantity());
    const double total = amount * product->sellingPrice();
    product->removeQuantity(amount);
    inventory.save();

    ofstream receipt(RECEIPT_FILE);
    receipt << "========================================\n"
            << "             SALES RECEIPT              \n"
            << "========================================\n"
            << "Product : " << product->getName() << '\n'
            << "ID      : " << product->getId() << '\n'
            << "Quantity: " << amount << '\n'
            << "Unit price: $" << fixed << setprecision(2) << product->sellingPrice() << '\n'
            << "Total   : $" << total << '\n'
            << "Remaining stock: " << product->getQuantity() << '\n'
            << "========================================\n";

    cout << "Sale completed. Receipt saved to " << RECEIPT_FILE << ".\n"
         << "Total: $" << fixed << setprecision(2) << total << '\n';
    if (product->isLowStock()) {
        cout << "LOW STOCK ALERT: reorder this product soon.\n";
    }
}

void printMenu() {
    cout << "\n========================================\n"
         << "       INVENTORY MANAGEMENT SYSTEM      \n"
         << "========================================\n"
         << "1. Add product\n2. Update product\n3. Delete product\n"
         << "4. Search product\n5. View inventory\n6. Inventory reports\n"
         << "7. Sell product / generate receipt\n8. Save inventory\n"
         << "9. Reload inventory\n0. Exit\n"
         << "========================================\n";
}

int main() {
    Inventory inventory;
    inventory.load();
    cout << "Inventory records loaded.\n";

    while (true) {
        printMenu();
        const int choice = readInteger("Choose an option: ", 0, 9);

        switch (choice) {
            case 1:
                addProduct(inventory);
                break;
            case 2:
                updateProduct(inventory);
                break;
            case 3:
                deleteProduct(inventory);
                break;
            case 4:
                inventory.search(readText("Enter product name or type: "));
                break;
            case 5:
                inventory.display();
                break;
            case 6:
                inventory.reports();
                break;
            case 7:
                sellProduct(inventory);
                break;
            case 8:
                cout << (inventory.save() ? "Inventory saved successfully.\n"
                                           : "Unable to save inventory.\n");
                break;
            case 9:
                inventory.load();
                cout << "Inventory reloaded successfully.\n";
                break;
            case 0:
                inventory.save();
                cout << "Inventory saved. Goodbye!\n";
                return 0;
        }
    }
}
