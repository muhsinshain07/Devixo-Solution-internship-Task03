# Inventory Management System

This project implements Task 03 of the Devixo Solutions C++ Programming Internship. It is a console-based inventory application built with classes, inheritance, polymorphism, file handling, vectors, and reusable functions.

## Features

The program supports adding, updating, deleting, and searching products. It displays quantity and price information, marks low-stock items, produces inventory reports, saves and reloads records automatically, and includes the bonus sales workflow.

The sales workflow reduces stock, calculates the correct selling price, creates `last_receipt.txt`, and displays a low-stock alert when the remaining quantity reaches the configured threshold.

## Object-oriented design

- `Product` is an abstract base class.
- `StandardProduct` and `DiscountedProduct` inherit from `Product`.
- Virtual functions provide polymorphic type names, file serialization, and selling-price behavior.
- `Inventory` owns a `vector<unique_ptr<Product>>` and manages the collection.

## Build

Use a C++17-compatible compiler:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic inventory_management_system.cpp -o inventory_management_system
```

## Run

Linux or macOS:

```bash
./inventory_management_system
```

Windows PowerShell:

```powershell
.\inventory_management_system.exe
```

## Menu options

1. Add product.
2. Update product.
3. Delete product.
4. Search product by name or type.
5. View the inventory table.
6. View total products, inventory value, available stock, and low-stock count.
7. Sell a product and generate a receipt.
8. Save inventory manually.
9. Reload inventory from disk.
0. Save and exit.

## Files created at runtime

- `inventory.txt` stores inventory records and is loaded automatically at startup.
- `last_receipt.txt` stores the most recent sales receipt.

Both files are created in the program's current working directory.
