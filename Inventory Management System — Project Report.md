# Inventory Management System — Project Report

## 1. Project overview

The Inventory Management System is a console application that manages a collection of products and stores the records permanently. It covers the requested product operations, inventory calculations, low-stock alerts, file handling, and a bonus sales receipt workflow.

## 2. Object-oriented structure

The abstract `Product` class contains the shared data and behavior for every inventory item. It stores the product ID, name, quantity, price, and reorder level. `StandardProduct` and `DiscountedProduct` derive from this base class. The discounted type overrides the selling-price calculation and file serialization, which demonstrates runtime polymorphism through virtual functions.

The `Inventory` class owns the products in a standard library vector of smart pointers. It is responsible for locating products, changing the collection, producing reports, and saving or loading records. Smart pointers ensure that product objects are released automatically when the inventory is destroyed.

## 3. Product and inventory operations

A product can be added only when its ID is unique. Product information can be updated without changing the ID, and a product can be removed by ID. The search operation checks the product name and product type. The inventory table shows the quantity, selling price, product type, and whether the item is at or below its reorder level.

The report provides the total number of products, the total number of units available, the total inventory value, and the number of low-stock products. For discounted products, the report uses the discounted selling price rather than the original listed price.

## 4. File handling

The application stores records in `inventory.txt`. Standard products use the `S` record code and discounted products use `D`. Each line contains the product type, ID, name, quantity, price, reorder level, and, for discounted items, the discount percentage. Invalid lines are ignored safely when loading. Inventory changes are saved automatically after add, update, delete, and sale operations.

## 5. Bonus sales feature

The sales option asks for a product ID and a quantity. It rejects quantities greater than the available stock, calculates the polymorphic selling price, reduces the inventory, saves the updated records, and writes a formatted receipt to `last_receipt.txt`. A low-stock message appears when the sale leaves the quantity at or below the product's threshold.

## 6. Requirement coverage

| Assignment requirement | Implementation |
| --- | --- |
| Classes and objects | `Product`, `StandardProduct`, `DiscountedProduct`, and `Inventory`. |
| Inheritance | Two concrete product classes inherit from `Product`. |
| Polymorphism | Virtual selling price, type, serialization, and display behavior. |
| File handling | Automatic save/load through `inventory.txt`. |
| STL vector | `vector<unique_ptr<Product>>` stores inventory items. |
| Product module | Add, update, delete, and search operations. |
| Inventory | Quantity, price, and low-stock threshold handling. |
| Reports | Product count, inventory value, available stock, and low-stock count. |
| Bonus | Sales receipt and quantity update after each sale. |

## 7. Testing

The application was compiled with C++17 using `-Wall`, `-Wextra`, and `-pedantic`. The test workflow covered standard and discounted product creation, inventory display, search, reports, update, sale, receipt creation, low-stock behavior, deletion, and reloading from the data file.

## 8. Conclusion

The project meets the task requirements using a maintainable class hierarchy and a small set of focused functions. Persistence and the sales workflow make the program more representative of a practical inventory application while keeping the interface suitable for a beginner-to-intermediate C++ project.
