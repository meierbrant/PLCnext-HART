<template>
<div>
    Detected Hart Gateways:
    <table class="table table-striped text-left">
        <tr>
            <th>Long Tag</th>
            <th>IP Address</th>
        </tr>
        <router-link :to="'/gateways/' + gw.serialNo" tag="tr"
                v-for="gw in gateways" v-bind:key="gw.serialNo" class="clickable">
            <td>
                {{ gw.longTag }}
            </td>
            <td>
                {{ gw.ip }}
            </td>
        </router-link>
    </table>

    <b-dropdown id="network-select" :text="selectedNetwork" variant="primary">
        <b-dropdown-item v-for="netinfo in networks" :key="netinfo.network" v-on:click="selectNetwork(netinfo)">{{ netinfo.network }}</b-dropdown-item>
    </b-dropdown>
</div>
</template>

<script lang="ts">
import { Vue, Component } from 'vue-property-decorator'
import { hartServerUrl, HartMuxDto, HartGw, updateInterval } from '../types'

interface NetworkInfo {
    name: string
    network: string
    addr: string
    bcast: string
    netmask: string
}

@Component({})
export default class HartGateways extends Vue {
    public networks: NetworkInfo[] = []
    public selectedNetwork: string = "Choose Another Network"
    public gateways: HartGw[] = []
    private polling: number = 0

    mounted () {
        this.discoverNetworks()
        this.updateGateways()
        this.polling = setInterval(this.updateGateways, updateInterval)
    }

    beforeDestroy () {
        clearInterval(this.polling)
    }

    discoverNetworks () {
        this.$http.get(hartServerUrl + '/networks').then(res => {
            this.networks = (res.data) as NetworkInfo[]
        })
    }

    public updateGateways () {
        this.$http.get(hartServerUrl + '/gw/discover').then(res => {
            this.gateways = res.data as HartGw[]
        })
    }

    public selectNetwork (netinfo: NetworkInfo) {
        this.selectedNetwork = netinfo.network
        this.$http.post(hartServerUrl + '/networks', {network: netinfo.network}).then(res => {
            this.updateGateways()
            clearInterval(this.polling)
            this.polling = setInterval(this.updateGateways, 5000)
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