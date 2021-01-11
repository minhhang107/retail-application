-- find_customer (customer_id IN NUMBER, found OUT NUMBER);

CREATE OR REPLACE PROCEDURE find_customer (customer_id IN NUMBER, found OUT NUMBER) AS  
    counter NUMBER := 0;
BEGIN 
    SELECT COUNT(*) INTO counter
    FROM customers
    WHERE customer_id = find_customer.customer_id;
    IF counter = 1 THEN
        found := 1;
    ELSE
        found := 0;
    END IF;
    
EXCEPTION
WHEN NO_DATA_FOUND 
    THEN
        found := 0;
WHEN OTHERS 
    THEN
        DBMS_OUTPUT.PUT_LINE('An error occurs');
END find_customer;

/

CREATE OR REPLACE PROCEDURE find_product (productId IN NUMBER, price OUT products.list_price%TYPE) AS 
BEGIN 
    SELECT list_price 
    INTO price 
    FROM products 
    WHERE product_id = productId; 
EXCEPTION 
    WHEN NO_DATA_FOUND 
        THEN 
            price := 0; 
    WHEN OTHERS 
        THEN 
            DBMS_OUTPUT.PUT_LINE('An error occurred!'); 
END find_product; 

/

CREATE TABLE new_orders  
AS SELECT * FROM orders; 

/

CREATE OR REPLACE PROCEDURE add_order (customer_id IN NUMBER, new_order_id OUT NUMBER) AS  
BEGIN 
    SELECT MAX(order_id) INTO new_order_id
    FROM new_orders;
    new_order_id := new_order_id + 1;
    
    INSERT INTO new_orders
    (order_id, customer_id, status, salesman_id, order_date)
    VALUES (new_order_id, customer_id, 'Shipped', 56, sysdate);  
EXCEPTION
WHEN TOO_MANY_ROWS
    THEN
        DBMS_OUTPUT.PUT_LINE('Too Many Rows Returned!');
WHEN NO_DATA_FOUND
    THEN
        DBMS_OUTPUT.PUT_LINE('No Data Found.');
WHEN OTHERS
    THEN 
        DBMS_OUTPUT.PUT_LINE('An Error Occurs!');  
END add_order; 

/

CREATE TABLE new_order_items
AS SELECT * FROM order_items; 

/

CREATE OR REPLACE PROCEDURE add_order_item ( 
    orderId IN new_order_items.order_id%TYPE, 
    itemId IN new_order_items.item_id%TYPE, 
    productId IN new_order_items.product_id%TYPE, 
    qty IN new_order_items.quantity%TYPE, 
    price IN new_order_items.unit_price%TYPE 
) AS 
BEGIN 
    INSERT INTO new_order_items (order_id, item_id, product_id, quantity, unit_price) 
    VALUES (orderId, itemId, productId, qty, price); 
EXCEPTION 
    WHEN OTHERS 
    THEN 
        DBMS_OUTPUT.PUT_LINE('An error occurred!'); 
END add_order_item; 

/