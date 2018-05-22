# Contribution guidelines 
Syncaide project accepts contributions via GitHub pull requests. This document outlines the process to help get your contribution accepted. The real goal is to give some perspective on the process and guidelines of contribution to try and avoid as much frustration and fruitless work as much as possible. As with anything else in this project, if there are ways you think this document can be imporoved, please feed free to either create an issue or just submit a pull request with your proposed changes to have a live discussion about those.

#### Issue reporting
It is very important to sometime have a thorough discussion before executing on the ideas. If you feel like there is a feature that needs to be added, just create an issue and rescribe your proposal. Remember that issues are open to anyone's comment so be ready to defend your idea. If you wish to have a private conversation just shoot me an email at costa@syncaide.com.

#### How to contribute to a repository
Begin by navigating to which ever represents the latest version branch being worked on. That will always be the only branch that has the version mark on it `v{major}.{minor}.{patch}`. e.g. `v0.0.2`
```bash
git checkout -b feature/{feature-name}
```
- feature name needs to have hyphens as namespace delimiter.
- branchs must start with the `feature/{}` folder separator.
- after first commit please create a pull request for other to help track your work.
- the pull request must be to the version branch, not to master.

#### Branch structure
_**master**_: This branch represents the complete working version of the software. Merging into this branch can only be done by a repository administrator and when done so, a version tag must be created to represent the version checkpoint.

_**v{version}**_: Version branchss are the running trunk branches. There is always going to be only one of these at a time representing the latest version being worked on.

_**feature/{feature-name}**_: These are the individual features being worked on. Any developer that works on any particular feature must work via a specific feature branch. These branches are pull requested into the latest version branches.
