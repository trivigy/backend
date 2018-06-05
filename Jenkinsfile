node {
    def customImage
    try {
        sh 'gcloud container clusters get-credentials us-east4-production-syncaide-com --zone us-east4-b'
        sh 'gcloud container clusters get-credentials us-east4-development-syncaide-com --zone us-east4-b'

        docker.image('gcr.io/syncaide-200904/builder:latest').inside {
            stage('Source') {
                checkout scm
            }
        }

        stage('Build') {
            customImage = docker.build('gcr.io/syncaide-200904/syncaide:latest')
        }

        stage('Package') {
            sh 'docker login -u oauth2accesstoken -p "$(gcloud auth print-access-token)" https://gcr.io'
            if (env.BRANCH_NAME == "master") {
                echo 'Package master'
                // customImage.push(env.TAG.substring(1))
            } else if (env.BRANCH_NAME ==~ "^v\\d+[.]\\d+[.]\\d+") {
                customImage.push()
            }
        }

        stage('Deploy') {
            sh 'helm init'
            sh 'helm plugin install https://github.com/nouney/helm-gcs'
            sh 'helm repo add syncaide gs://static.syncaide.com/charts'
            sh 'helm repo update'

            if (env.BRANCH_NAME == "master") {
                echo 'Deploy production'
                // helm install --name landpage landpage --set image.repositoryOverride="$(docker inspect gcr.io/syncaide-200904/landpage:${env.BRANCH_NAME.substring(1)} | jq -rcM '.[].RepoDigests[0]')"
            } else if (env.BRANCH_NAME ==~ "^v\\d+[.]\\d+[.]\\d+") {
                echo 'Deploy staging'
                // sh 'kubectl config use-context gke_syncaide-200904_us-east4-b_us-east4-production-syncaide-com'
                // sh 'helm upgrade landpage syncaide/landpage --set image.repositoryOverride="$(docker inspect gcr.io/syncaide-200904/landpage:latest | jq -rcM ".[].RepoDigests[0]")"'
            }
        }
        currentBuild.result = 'SUCCESS'
    } catch (error) {
        currentBuild.result = 'FAILURE'
    } finally {
        def url = sh(returnStdout: true, script: 'git config remote.origin.url').trim()
        def author = sh(returnStdout: true, script: 'git show -s --pretty=%an').trim()
        def repo = (url =~ /((?:[^\/:]+)[\/](?:[^\/]+))[.]git$/)[0].getAt(1).trim()

        discordSend(
            title: "${repo} pipeline",
            successful: currentBuild.resultIsBetterOrEqualTo('SUCCESS'),
            link: env.RUN_DISPLAY_URL,
            description: """
**branch**: `${env.BRANCH_NAME}`
**build**: ${env.BUILD_DISPLAY_NAME}
**status**: ${currentBuild.result}
**duration**: ${currentBuild.duration}
            """,
            footer: author,
            webhookURL: 'https://discordapp.com/api/webhooks/451573270961979392/7WJ1izyhGTUKqach_awSEh9HsLerRUbbIEpmb2HqOj4eOWd-CyuQWmfXvFE00Eg2TgUd'
        )
    }
}
