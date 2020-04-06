<template>
<table class="table table-striped">
    <router-link :to="'/gateways/' + gw.serialNo" v-for="gw in gateways" v-bind:key="gw.serialNo">
        <tr>
            {{ gw.ip }}
        </tr>
    </router-link>
</table>
</template>

<script lang="ts">
import { Vue, Component } from 'vue-property-decorator'
import { hartServerUrl, HartMuxDto, HartGw, updateInterval } from '../types'

@Component({})
export default class HartGateways extends Vue {
    public gateways: HartGw[] = []
    private polling: number = 0

    mounted () {
        this.updateGateways()
        this.polling = setInterval(this.updateGateways, updateInterval)
    }

    beforeDestroy () {
        clearInterval(this.polling)
    }

    public updateGateways () {
        this.$http.get(hartServerUrl + '/gw/discover').then(res => {
            this.gateways = res.data as HartGw[]
        })
    }
}
</script>

<style lang="scss">
.disabled, .disabled * {
    background-color: #666 !important;
    border-color: #666 !important;
    cursor: default;
}
</style>