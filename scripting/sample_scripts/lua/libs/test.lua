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