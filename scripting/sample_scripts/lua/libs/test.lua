--[[
Copyright (c) 2018 Red Koi Box

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Author: Fabio Greotti (f.greotti@redkoibox.it)
--]]

return
{
    tests = {},

    registerTest = function(self, name, func)
        self.tests[name] = func;
    end,

    runTest = function(self, testName, json)
        local results = { success = true, tests = {} };
        if(testName == nil or test == "") then
            for k, v in pairs(self.tests) do
                local res, details = v(json);
                results.tests[k] = { result = res, desc = details };
                if(res == false) then
                    results.success = false;
                end
            end
        else
            local testMethod = self.tests[testName];
            if(testMethod ~= nil) then
                local res, details = testMethod(json);
                results.tests[testName] = { result = res, desc = details };
                results.success = res;
            end
        end
        return results.success
                and 
            Helix.success(results.tests)
                or
            Helix.error(Helix.Errors.SERVER_ERROR, results.tests);
    end
};