// Find and replace
cat input.txt | sed 's/find/replace/g' > output.txt

// Sort
sort input.txt -o output.txt

// Convert line return
awk '{ sub("\r$", ""); print }' input.txt > output.txt

// Execute sql query from file.
drush sqlq --file=input.sql
