
#include <iostream>
#include <occi.h>

using oracle::occi::Environment;
using oracle::occi::Connection;
using namespace oracle::occi;
using namespace std;

struct ShoppingCart {
	int product_id;
	double price;
	int quantity;
};

int mainMenu() {
	int selection = 0;

	do {
		cout << "******************** Main Menu ********************" << endl;
		cout << "1)\tLogin\n";
		cout << "0)\tExit\n";

		if (selection != 0 && selection != 1) {
			cout << "You entered a wrong value. Enter an option (0-1): ";
		}
		else {
			cout << "Enter an option (0-1): ";
		}
		cin >> selection;
	} while (selection != 0 && selection != 1);

	return selection;
}


int customerLogin(Connection* conn, int customerId) {
    int found = 0;
    // Define the statement
    Statement* statement = conn->createStatement("BEGIN find_customer(:1, :2); END;");

    try {
        // Set the first parameter
        statement->setInt(1, customerId);
        // Set the second parameter
        statement->registerOutParam(2, Type::OCCIINT, sizeof(found));
        // Execute the procedure
        statement->executeQuery();
        // Save the returning value
        found = statement->getInt(2);
    }
    catch (SQLException & sqlExcp) {
        cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
    }

    // Commit the transaction
    conn->commit();
    // Terminate the statement
    conn->terminateStatement(statement);

    return found;
}


double findProduct(Connection* conn, int product_id) {
    double price = 0.0;
    // Define the statement
    Statement* statement = conn->createStatement("BEGIN find_product(:1, :2); END;");

    try {
        // Set the first parameter
        statement->setInt(1, product_id);
        // Set the second parameter
        statement->registerOutParam(2, Type::OCCIDOUBLE, sizeof(price));
        // Execute the procedure
        statement->executeQuery();
        // Save the returning price
        price = statement->getDouble(2);
        // Commit
        conn->commit();
    }
    catch (SQLException & sqlExcp) {
        cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
    }
    catch (exception & excp)
    {
        cerr << excp.what() << endl;
    }

    // Terminate the statement
    conn->terminateStatement(statement);

    return price;
}

int addToCart(Connection* conn, struct ShoppingCart cart[]) {
    const int maxItem = 5;
    int moreProduct = 1;
    int count = 0;

    cout << "-------------- Add Products to Cart --------------" << endl;
    for (int i = 0; i < maxItem && moreProduct == 1; i++) {
        do {
            cout << "Enter the product ID: ";
            cin >> cart[i].product_id;
            cart[i].price = findProduct(conn, cart[i].product_id);
            if (cart[i].price == 0) {
                cout << "The product does not exists. Try again..." << endl;
            }
            else {
                cout << "Product Price: " << cart[i].price << endl;
                cout << "Enter the product Quantity: ";
                cin >> cart[i].quantity;
                ++count;
                cout << "Enter 1 to add more products or 0 to checkout: ";
                cin >> moreProduct;
            }
        } while (cart[i].price == 0);
    }
    return count;
};


void displayProducts(struct ShoppingCart cart[], int productCount) {
    double total = 0;
    cout << "------- Ordered Products ---------" << endl;
    for (int i = 0; i < productCount; i++) {
        cout << "---Item " << i + 1 << endl;
        cout << "Product ID: " << cart[i].product_id << endl;
        cout << "Price: " << cart[i].price << endl;
        cout << "Quantity: " << cart[i].quantity << endl;
        total += (cart[i].price * cart[i].quantity);
    }
    cout << "----------------------------------" << endl;
    cout << "Total: " << total << endl;
};


int checkout(Connection* conn, struct ShoppingCart cart[], int customerId, int productCount) {
    char option;

    do {
        cout << "Would you like to checkout? (Y/y or N/n) ";
        cin >> option;
        if (tolower(option) != 'y' && tolower(option) != 'n')
            cout << "Wrong input. Try again..." << endl;
    } while (tolower(option) != 'y' && tolower(option) != 'n');

    if (tolower(option) == 'y') {
        int orderId;
        // Define the statement
        Statement* statement = conn->createStatement();

        try {
            // Setting the procedure for adding an order
            statement->setSQL("BEGIN add_order(:1, :2); END;");
            // Set the customer ID
            statement->setInt(1, customerId);
            // Register the new order ID parameter
            statement->registerOutParam(2, Type::OCCIINT, sizeof(orderId));
            // INSERT an order
            statement->executeUpdate();
            // Save the returning order ID
            orderId = statement->getInt(2);

            // Setting the procedure for adding an item
            statement->setSQL("BEGIN add_order_item(:1, :2, :3, :4, :5); END;");
            // Set the order ID
            statement->setInt(1, orderId);
            // Add each product
            for (int item = 0; item < productCount; item++) {
                // Set the item ID
                statement->setInt(2, item + 1);
                // Set the product ID
                statement->setInt(3, cart[item].product_id);
                // Set the product quantity
                statement->setInt(4, cart[item].quantity);
                // Set the product price
                statement->setDouble(5, cart[item].price);
                // INSERT an order item
                statement->executeUpdate();
                // Commit transaction
                conn->commit();
            }
        }
        catch (SQLException & sqlExcp) {
            cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
        }
        catch (exception & excp)
        {
            cerr << excp.what() << endl;
        }

        // Terminate the statement
        conn->terminateStatement(statement);
        return 1;
    }
    else
        return 0;
}


int main()
{
    // The state of the program
    int keepRunning = 1;

    // OCCI Variables
    Environment* env = nullptr;
    Connection* conn = nullptr;

    // Login Credentials
    string user = "dbs311_203b19";
    string pass = "30794296";
    string conStr = "myoracle12c.senecacollege.ca:1521/oracle12c";

    try {
        // Create environment
        env = Environment::createEnvironment(Environment::DEFAULT);
        // Open a connection
        conn = env->createConnection(user, pass, connStr);

        while (keepRunning) {
            keepRunning = mainMenu();
            if (keepRunning == 1) {
                int customerId;
                cout << "Enter the customer ID: ";
                cin >> customerId;
                if (customerLogin(conn, customerId) == 1) {
                    ShoppingCart cart[5];

                    // The number of ordered items
                    int productCount = addToCart(conn, cart);
                    displayProducts(cart, productCount);
                    if (checkout(conn, cart, customerId, productCount))
                        cout << "The order is successfully completed." << endl;
                    else
                        cout << "The order is cancelled." << endl;
                }
                else
                    cout << "The customer does not exist." << endl;
            }
            else
                cout << "Good bye!..." << endl;
        }

        // Close the connection
        env->terminateConnection(conn);
        Environment::terminateEnvironment(env);
    }
    catch (SQLException & sqlExcp) {
        cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
    }

    return 0;
}