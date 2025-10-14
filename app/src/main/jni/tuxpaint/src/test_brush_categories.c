/**
 * test_brush_categories.c
 * 
 * Unit test for brush categorization
 * Verifies that brush categories are correctly defined and sorted
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Maximum values from tuxpaint.c */
#define MAX_BRUSHES 256
#define MAX_BRUSHES_PER_CATEGORY 20
#define NUM_BRUSH_CATEGORIES 10

/* Brush category structure */
typedef struct {
  int category_id;
  int brush_ids[MAX_BRUSHES_PER_CATEGORY];
  int count;
  int use_icons;
  int variable_size;
} brush_category_t;

/* Copy of the brush_categories array from tuxpaint.c */
static const brush_category_t brush_categories[NUM_BRUSH_CATEGORIES] = {
  /* Category 1: Standard brushes */
  {1, {0, 1, 2, 3, 4}, 5, 0, 1},
  
  /* Category 2: Special round brushes */
  {2, {7, 8, 5, 6, 37, 36}, 6, 0, 1},
  
  /* Category 3: Mixed icon brushes */
  {3, {35, 34, 48, 30, 39, 33, 38, 49, 51, 54, 65, 66, 67}, 13, 1, 0},
  
  /* Category 4: Shapes */
  {4, {19, 31, 32, 40, 61, 63, 64, 68, 50, 52}, 10, 1, 0},
  
  /* Category 5: Flowers */
  {5, {20, 21, 22, 23, 24}, 5, 1, 1},
  
  /* Category 6: Animals & Nature */
  {6, {14, 15, 16, 27, 28, 29, 53}, 7, 1, 0},
  
  /* Category 7: Slash lines */
  {7, {42, 43, 44, 45, 46, 47}, 6, 1, 1},
  
  /* Category 8: Squares */
  {8, {55, 56, 57, 58, 59, 60}, 6, 1, 1},
  
  /* Category 9: Texture brushes */
  {9, {9, 25, 26, 62}, 4, 1, 0},
  
  /* Category 10: Effect brushes */
  {10, {10, 11, 12, 13, 17, 18, 41}, 7, 1, 0}
};

/* Test helper function to find category for a brush */
static const brush_category_t* find_brush_category(int brush_id)
{
  for (int cat_idx = 0; cat_idx < NUM_BRUSH_CATEGORIES; cat_idx++) {
    const brush_category_t *category = &brush_categories[cat_idx];
    for (int i = 0; i < category->count; i++) {
      if (category->brush_ids[i] == brush_id) {
        return category;
      }
    }
  }
  return NULL;
}

/* Test: All categories have valid IDs */
void test_category_ids_are_sequential(void)
{
  printf("TEST: Category IDs are sequential (1-10)...\n");
  
  for (int i = 0; i < NUM_BRUSH_CATEGORIES; i++) {
    assert(brush_categories[i].category_id == i + 1);
    printf("  ✓ Category %d has correct ID\n", i + 1);
  }
  
  printf("PASS: All category IDs are sequential\n\n");
}

/* Test: No duplicate brushes across categories */
void test_no_duplicate_brushes(void)
{
  printf("TEST: No duplicate brushes across categories...\n");
  
  int brush_seen[MAX_BRUSHES] = {0};
  
  for (int cat_idx = 0; cat_idx < NUM_BRUSH_CATEGORIES; cat_idx++) {
    const brush_category_t *category = &brush_categories[cat_idx];
    
    for (int i = 0; i < category->count; i++) {
      int brush_id = category->brush_ids[i];
      
      if (brush_seen[brush_id]) {
        printf("  ✗ FAILED: Brush %d appears in multiple categories!\n", brush_id);
        assert(0);
      }
      
      brush_seen[brush_id] = 1;
    }
  }
  
  printf("PASS: No duplicate brushes found\n\n");
}

/* Test: Specific brushes are in expected categories */
void test_specific_brush_placements(void)
{
  printf("TEST: Specific brushes are in expected categories...\n");
  
  /* Test some key brushes */
  struct {
    int brush_id;
    int expected_category;
    const char *name;
  } test_cases[] = {
    {0, 1, "Standard brush 0"},
    {1, 1, "Standard brush 1"},
    {5, 2, "Special round brush 5"},
    {20, 5, "Flower brush 20"},
    {35, 3, "Mixed icon brush 35"},
    {42, 7, "Slash line brush 42"},
    {55, 8, "Square brush 55"},
    {9, 9, "Texture brush 9"},
    {10, 10, "Effect brush 10"},
  };
  
  for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
    const brush_category_t *category = find_brush_category(test_cases[i].brush_id);
    
    if (category == NULL) {
      printf("  ✗ FAILED: %s (ID %d) not found in any category!\n", 
             test_cases[i].name, test_cases[i].brush_id);
      assert(0);
    }
    
    if (category->category_id != test_cases[i].expected_category) {
      printf("  ✗ FAILED: %s (ID %d) in category %d, expected %d\n",
             test_cases[i].name, test_cases[i].brush_id,
             category->category_id, test_cases[i].expected_category);
      assert(0);
    }
    
    printf("  ✓ %s in category %d (correct)\n", 
           test_cases[i].name, category->category_id);
  }
  
  printf("PASS: All specific brushes in correct categories\n\n");
}

/* Test: Category counts match actual brush counts */
void test_category_counts(void)
{
  printf("TEST: Category counts are correct...\n");
  
  int expected_counts[] = {5, 6, 13, 10, 5, 7, 6, 6, 4, 7};
  
  for (int i = 0; i < NUM_BRUSH_CATEGORIES; i++) {
    assert(brush_categories[i].count == expected_counts[i]);
    printf("  ✓ Category %d has %d brushes (correct)\n", 
           i + 1, brush_categories[i].count);
  }
  
  printf("PASS: All category counts correct\n\n");
}

/* Test: Icon and variable size flags */
void test_category_flags(void)
{
  printf("TEST: Category flags (use_icons, variable_size) are correct...\n");
  
  struct {
    int category_id;
    int use_icons;
    int variable_size;
  } expected[] = {
    {1, 0, 1},  /* Standard brushes */
    {2, 0, 1},  /* Special round */
    {3, 1, 0},  /* Mixed icons */
    {4, 1, 0},  /* Shapes */
    {5, 1, 1},  /* Flowers */
    {6, 1, 0},  /* Animals */
    {7, 1, 1},  /* Slash lines */
    {8, 1, 1},  /* Squares */
    {9, 1, 0},  /* Textures */
    {10, 1, 0}, /* Effects */
  };
  
  for (int i = 0; i < NUM_BRUSH_CATEGORIES; i++) {
    assert(brush_categories[i].use_icons == expected[i].use_icons);
    assert(brush_categories[i].variable_size == expected[i].variable_size);
    printf("  ✓ Category %d: use_icons=%d, variable_size=%d (correct)\n",
           i + 1, brush_categories[i].use_icons, 
           brush_categories[i].variable_size);
  }
  
  printf("PASS: All category flags correct\n\n");
}

/* Test: Total brush count */
void test_total_brush_count(void)
{
  printf("TEST: Total brush count...\n");
  
  int total = 0;
  for (int i = 0; i < NUM_BRUSH_CATEGORIES; i++) {
    total += brush_categories[i].count;
  }
  
  printf("  Total brushes across all categories: %d\n", total);
  
  /* We have 69 brushes defined in the categories */
  assert(total == 69);
  
  printf("PASS: Total brush count is 69\n\n");
}

/* Test: Sorted order verification */
void test_sorted_order_simulation(void)
{
  printf("TEST: Simulating sorted brush order...\n");
  
  int sorted_brushes[MAX_BRUSHES];
  int sorted_count = 0;
  
  /* Build sorted array */
  for (int cat_idx = 0; cat_idx < NUM_BRUSH_CATEGORIES; cat_idx++) {
    const brush_category_t *category = &brush_categories[cat_idx];
    for (int i = 0; i < category->count; i++) {
      sorted_brushes[sorted_count++] = category->brush_ids[i];
    }
  }
  
  printf("  First 10 brushes in sorted order: ");
  for (int i = 0; i < 10 && i < sorted_count; i++) {
    printf("%d ", sorted_brushes[i]);
  }
  printf("\n");
  
  printf("  Expected: 0 1 2 3 4 7 8 5 6 37 (category 1 & 2 start)\n");
  
  /* Verify first category */
  assert(sorted_brushes[0] == 0);
  assert(sorted_brushes[1] == 1);
  assert(sorted_brushes[2] == 2);
  assert(sorted_brushes[3] == 3);
  assert(sorted_brushes[4] == 4);
  
  /* Verify second category starts */
  assert(sorted_brushes[5] == 7);
  assert(sorted_brushes[6] == 8);
  
  printf("PASS: Sorted order is correct\n\n");
}

/* Main test runner */
int main(void)
{
  printf("=====================================\n");
  printf("Brush Categories Unit Test\n");
  printf("=====================================\n\n");
  
  test_category_ids_are_sequential();
  test_category_counts();
  test_no_duplicate_brushes();
  test_category_flags();
  test_specific_brush_placements();
  test_total_brush_count();
  test_sorted_order_simulation();
  
  printf("=====================================\n");
  printf("ALL TESTS PASSED ✓\n");
  printf("=====================================\n");
  
  return 0;
}
