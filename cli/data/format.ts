let readme = Deno.readTextFileSync('helplines.txt');

const escapechars = ['t','n','r'];

escapechars.forEach((char) => readme = readme.replace(new RegExp('\\' + char, 'g'), '\\' + char));

Deno.writeTextFileSync('headerString.txt', readme)
