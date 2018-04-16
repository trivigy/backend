Repository Contribution guldeline :

[backend]

-- master branch : only version branch can be merged to master branch

-- version branch : from the version branch creates your own feature branch

```
git checkout - b feature/new-feature 
```

-- feature branch merges to version branch (note : please only fork from version branch)



When commit your contribution, please use the following steps:

`git commit`

`git checkout -b feature/new-features (checkout from version branch)`

git push (set origin if new feature branch/does not exist in remote repository )

`create pulll request to version branch`

When creating the pull request, the repository/github UI will run unit test

