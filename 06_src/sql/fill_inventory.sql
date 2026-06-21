-- 批量补库存：一键执行
BEGIN;

-- 对于组件 1~36（seed 中的标准元器件），给足库存
UPDATE Inventory SET quantity = 500  WHERE component_id = 1;
UPDATE Inventory SET quantity = 2000 WHERE component_id = 2;
UPDATE Inventory SET quantity = 1000 WHERE component_id = 3;
UPDATE Inventory SET quantity = 800  WHERE component_id = 4;
UPDATE Inventory SET quantity = 600  WHERE component_id = 5;
UPDATE Inventory SET quantity = 5000 WHERE component_id = 6;
UPDATE Inventory SET quantity = 5000 WHERE component_id = 7;
UPDATE Inventory SET quantity = 5000 WHERE component_id = 8;
UPDATE Inventory SET quantity = 3000 WHERE component_id = 9;
UPDATE Inventory SET quantity = 3000 WHERE component_id = 10;
UPDATE Inventory SET quantity = 3000 WHERE component_id = 11;
UPDATE Inventory SET quantity = 3000 WHERE component_id = 12;
UPDATE Inventory SET quantity = 500  WHERE component_id = 13;
UPDATE Inventory SET quantity = 500  WHERE component_id = 14;
UPDATE Inventory SET quantity = 500  WHERE component_id = 15;
UPDATE Inventory SET quantity = 500  WHERE component_id = 16;
UPDATE Inventory SET quantity = 200  WHERE component_id = 17;
UPDATE Inventory SET quantity = 200  WHERE component_id = 18;
UPDATE Inventory SET quantity = 200  WHERE component_id = 19;
UPDATE Inventory SET quantity = 200  WHERE component_id = 20;
UPDATE Inventory SET quantity = 100  WHERE component_id = 21;
UPDATE Inventory SET quantity = 100  WHERE component_id = 22;
UPDATE Inventory SET quantity = 300  WHERE component_id = 23;
UPDATE Inventory SET quantity = 300  WHERE component_id = 24;
UPDATE Inventory SET quantity = 300  WHERE component_id = 25;
UPDATE Inventory SET quantity = 300  WHERE component_id = 26;
UPDATE Inventory SET quantity = 1000 WHERE component_id = 27;
UPDATE Inventory SET quantity = 500  WHERE component_id = 28;
UPDATE Inventory SET quantity = 200  WHERE component_id = 29;
UPDATE Inventory SET quantity = 500  WHERE component_id = 30;
UPDATE Inventory SET quantity = 500  WHERE component_id = 31;
UPDATE Inventory SET quantity = 500  WHERE component_id = 32;
UPDATE Inventory SET quantity = 500  WHERE component_id = 33;
UPDATE Inventory SET quantity = 500  WHERE component_id = 34;
UPDATE Inventory SET quantity = 500  WHERE component_id = 35;
UPDATE Inventory SET quantity = 500  WHERE component_id = 36;

-- BOM 导入的元器件（37+）：补一条 Inventory 记录并给库存
INSERT INTO Inventory (component_id, warehouse_id, quantity)
SELECT c.component_id, 1, 
    CASE 
        WHEN c.name ILIKE '%Ω%'  THEN 5000
        WHEN c.name ILIKE '%μF%' THEN 2000
        WHEN c.name ILIKE '%uF%' THEN 2000
        WHEN c.name ILIKE '%nF%' THEN 2000
        WHEN c.name ILIKE '%pF%' THEN 2000
        WHEN c.name ILIKE '%uH%' THEN 1000
        ELSE 500
    END
FROM Component c
WHERE c.component_id >= 37
  AND NOT EXISTS (SELECT 1 FROM Inventory i WHERE i.component_id = c.component_id);

-- 已有 Inventory 记录但 quantity=0 的也填充
UPDATE Inventory SET quantity = 2000
WHERE component_id >= 37 AND quantity = 0
  AND EXISTS (SELECT 1 FROM Component c WHERE c.component_id = Inventory.component_id
              AND (c.name ILIKE '%μF%' OR c.name ILIKE '%uF%' OR c.name ILIKE '%nF%' OR c.name ILIKE '%pF%'));

UPDATE Inventory SET quantity = 5000
WHERE component_id >= 37 AND quantity = 0
  AND EXISTS (SELECT 1 FROM Component c WHERE c.component_id = Inventory.component_id
              AND c.name ILIKE '%Ω%');

UPDATE Inventory SET quantity = 1000
WHERE component_id >= 37 AND quantity = 0;

COMMIT;

SELECT component_id, c.name, i.quantity
FROM Inventory i JOIN Component c ON i.component_id = c.component_id
WHERE i.warehouse_id = 1 AND i.quantity < 100
ORDER BY component_id;
